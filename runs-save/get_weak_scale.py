import pandas as pd
import json
import matplotlib.pyplot as plt
import numpy as np
df = pd.read_csv("data/project.csv").sort_index(axis=1)
df['config.is'] = df['config.iw'] * df['config.ih']
scaling_sweep = df['sweep_id'] == 'scaling'
scaling_df = df[scaling_sweep]
scaling_parallel_df = scaling_df[scaling_df['config.n'] > 0]
scaling_sequential_df = scaling_df[scaling_df['config.n'] < 0]
scaling_parallel_results = scaling_parallel_df.drop(columns=['config.rle', 'name', 'sweep_id', 'config.host', 'summary._wandb']).groupby(['config.is', 'config.n']).mean()
scaling_sequential_results = scaling_sequential_df.drop(columns=['config.rle', 'name', 'sweep_id', 'config.host', 'summary._wandb']).groupby(['config.is', 'config.n']).mean()

scaling_sequential_results['its/ms'] = scaling_sequential_results['summary.process.0.results.iterations'] / scaling_sequential_results['summary.process.0.results.time.total']
scaling_parallel_results['its/ms'] = scaling_parallel_results['summary.process.0.results.iterations'] / scaling_parallel_results['summary.process.0.results.time.total']


def ss_1():
    sequential_reset = scaling_sequential_results.reset_index().set_index(['config.is'])
    parallel_reset = scaling_parallel_results.reset_index().set_index(['config.is'])
    merged_results = parallel_reset.merge(
        sequential_reset,
        left_index=True,
        right_index=True,
        suffixes=('_parallel', '_sequential')
    ).reset_index()
    merged_results['speedup'] = merged_results['its/ms_parallel'] / merged_results['its/ms_sequential']
    fig, ax = plt.subplots()#figsize=(14, 8))
    for (_is,), group in merged_results.groupby(['config.is']):
        ax.plot(group['config.n_parallel'], group['speedup'], label=f"{_is}", marker='o')
    n_values = np.sort(merged_results['config.n_parallel'].unique())
    linear_reference = [1 for x in n_values]  # Linear speedup reference
    plot_limits = ax.get_xlim(), ax.get_ylim()
    ax.plot(n_values, linear_reference, label='Linear Reference', linestyle='--', color='red')
    ax.set_xlim(plot_limits[0])
    ax.set_ylim(plot_limits[1])
    ax.set_xlabel('Number of Processes (n)')
    ax.set_ylabel('Efficiency')
    ax.legend(title='Cells per process', loc='upper left', bbox_to_anchor=(1, 1))
    plt.tight_layout()
    plt.savefig("data/weak_scale_1.png")

def ss_2():
    sequential_reset = scaling_sequential_results.reset_index().set_index(['config.is'])
    parallel_reset = scaling_parallel_results.reset_index().set_index(['config.is'])
    merged_results = parallel_reset.merge(
        sequential_reset,
        left_index=True,
        right_index=True,
        suffixes=('_parallel', '_sequential')
    ).reset_index()
    merged_results['speedup'] = (merged_results['its/ms_parallel'] / merged_results['its/ms_sequential']) * \
                                merged_results['config.n_parallel']
    fig, ax = plt.subplots()#figsize=(14, 8))
    for (_is,), group in merged_results.groupby(['config.is']):
        ax.plot(group['config.n_parallel'], group['speedup'], label=f"{_is}", marker='o')
    n_values = np.sort(merged_results['config.n_parallel'].unique())
    linear_reference = [x for x in n_values]  # Linear speedup reference
    plot_limits = ax.get_xlim(), ax.get_ylim()
    ax.plot(n_values, linear_reference, label='Linear Reference', linestyle='--', color='red')
    ax.set_xlim(plot_limits[0])
    ax.set_ylim(plot_limits[1])
    ax.set_xlabel('Number of Processes (n)')
    ax.set_ylabel('Speedup')
    ax.legend(title='Cells per process', loc='upper left', bbox_to_anchor=(1, 1))
    plt.tight_layout()
    plt.savefig("data/weak_scale_2.png")

ss_1()
ss_2()