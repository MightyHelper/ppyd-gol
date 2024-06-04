import pandas as pd
import json
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

df = pd.read_csv("data/project.csv").sort_index(axis=1)

df['config.os'] = 144 * df['config.os_index'] ** 2
scaling_sweep = (df['sweep_id'] == 'scaling3.2')
parallel = df['config.n'] > 0
scaling_df = df[scaling_sweep]
# for column, c in zip(scaling_df.dtypes, scaling_df.columns):
#     print(column, c)
scaling_parallel_df = scaling_df
scaling_parallel_results = scaling_parallel_df.drop(columns=[
    'config.rle',
    'name',
    'config.host',
    'summary._wandb',
    'run_config.environment.hostname',
    'run_config.environment.program_type',
    'run_config.file',
    'summary.run_config.file',
    'summary.run_config.environment.program_type',
    'summary.run_config.environment.hostname'
]).groupby(['config.os', 'config.n', 'sweep_id']).mean()
scaling_parallel_results['ms/its'] = scaling_parallel_results['process.0.results.time.total'] / scaling_parallel_results['process.0.results.iterations']

par = scaling_parallel_results.reset_index()
n_values = set(par['config.n'])
n_values2 = set(par['config.os'])
palette = sns.color_palette("nipy_spectral", len(n_values))

# X: log(outer size)
# Y: log(ms/its)

fig, ax = plt.subplots()
for i, x in enumerate(n_values):
    subset = par[par['config.n'] == x]
    ax.plot(subset['config.os'], subset['ms/its'], color=palette[i], label=f'{x}' if x > 0 else 'Sequential')
    ax.scatter(subset['config.os'], subset['ms/its'], color=palette[i])
ax.legend(title='Nodes')
ax.set_xlabel('Outer Size')
ax.set_ylabel('Milliseconds per iteration')
ax.set_yscale('log')
ax.set_xscale('log')
plt.savefig("data/linear_in_os.png")

def time_per_it(o_size, n):
    items = (par['config.os'] == o_size) & (par['config.n'] == n)
    assert items.sum() > 0, f"No data for outer size {o_size} and {n} nodes"
    result = par[items]['ms/its'].values[0]
    # print(f"Time per iteration for outer size {o_size} and {n} nodes: {result}")
    return result

def speedup(o_size, n):
    s = time_per_it(o_size, -1) / time_per_it(o_size, n)
    # print(f"Speedup for outer size {o_size} and {n} nodes: {s}")
    return s

def efficiency(o_size, n):
    e = speedup(o_size, n) / n
    # print(f"Efficiency for outer size {o_size} and {n} nodes: {e}")
    return e

def plot_speedup():
    palette = sns.color_palette("nipy_spectral", len(n_values))
    fig, ax = plt.subplots()
    os_values = list(sorted(set(par['config.os'])))
    for i, x in enumerate(n_values):
        if x < 0:
            continue
        ax.plot(os_values, [speedup(o_size, x) for o_size in os_values], color=palette[i], label=f'{x}' if x > 0 else 'Sequential')
        ax.scatter(os_values, [speedup(o_size, x) for o_size in os_values], color=palette[i])
    plot_limits = ax.get_xlim(), ax.get_ylim()
    linear_reference = [x for x in os_values]
    ax.plot(os_values, linear_reference, label='Linear Reference', linestyle='--', color='red')
    ax.legend(title='Nodes', loc='upper left', bbox_to_anchor=(1, 1))
    ax.set_xlim(plot_limits[0])
    ax.set_ylim(plot_limits[1])
    ax.set_xlabel('Outer Size')
    ax.set_ylabel('Speedup')
    plt.tight_layout()
    plt.savefig("data/speedup_in_os.png")

def plot_efficiency():
    palette = sns.color_palette("nipy_spectral", len(n_values))
    fig, ax = plt.subplots()
    os_values = list(sorted(set(par['config.os'])))
    for i, x in enumerate(n_values):
        if x < 0:
            continue
        ax.plot(os_values, [efficiency(o_size, x) for o_size in os_values], color=palette[i], label=f'{x}' if x > 0 else 'Sequential')
        ax.scatter(os_values, [efficiency(o_size, x) for o_size in os_values], color=palette[i])
    plot_limits = ax.get_xlim(), ax.get_ylim()
    linear_reference = [1 for x in os_values]
    ax.plot(os_values, linear_reference, label='Linear Reference', linestyle='--', color='red')
    ax.set_xlim(plot_limits[0])
    ax.set_ylim(0, 1.01)
    ax.legend(title='Nodes', loc='upper left', bbox_to_anchor=(1, 1))
    ax.set_xlabel('Outer Size')
    ax.set_ylabel('Efficiency')
    plt.tight_layout()
    plt.savefig("data/efficiency_in_os.png")

def plot_efficiency2():
    palette = sns.color_palette("coolwarm", len(n_values2))
    fig, ax = plt.subplots()
    os_values = list(sorted(set(par['config.os'])))
    positive_n_values = list(sorted(set(par[par['config.n'] > 0]['config.n'])))
    for i, x in enumerate(os_values):
        values_ = [efficiency(x, n) for n in positive_n_values]
        ax.plot(positive_n_values, values_, color=palette[i], label=f'{x}')
        ax.scatter(positive_n_values, values_, color=palette[i])
    plot_limits = ax.get_xlim(), ax.get_ylim()
    linear_reference = [1 for x in positive_n_values]
    ax.plot(positive_n_values, linear_reference, label='Linear Reference', linestyle='--', color='red')
    ax.set_xlim(plot_limits[0])
    ax.set_ylim(0, 1.01)
    ax.legend(title='Outer size', loc='upper left', bbox_to_anchor=(1, 1))
    ax.set_xlabel('Nodes')
    ax.set_ylabel('Efficiency')
    plt.tight_layout()
    plt.savefig("data/true_efficiency_in_os.png")

def plot_speedup2():
    palette = sns.color_palette("coolwarm", len(n_values2))
    fig, ax = plt.subplots()
    os_values = list(sorted(set(par['config.os'])))
    positive_n_values = list(sorted(set(par[par['config.n'] > 0]['config.n'])))
    for i, x in enumerate(os_values):
        values_ = [speedup(x, n) for n in positive_n_values]
        ax.plot(positive_n_values, values_, color=palette[i], label=f'{x}')
        ax.scatter(positive_n_values, values_, color=palette[i])
    # Add linear reference
    plot_limits = ax.get_xlim(), ax.get_ylim()
    linear_reference = [x for x in positive_n_values]
    ax.plot(positive_n_values, linear_reference, label='Linear Reference', linestyle='--', color='red')
    ax.set_xlim(plot_limits[0])
    ax.set_ylim(plot_limits[1])
    ax.legend(title='Outer size', loc='upper left', bbox_to_anchor=(1, 1))
    ax.set_xlabel('Nodes')
    ax.set_ylabel('Speedup')
    plt.tight_layout()
    plt.savefig("data/true_speedup_in_os.png")

plot_speedup()
plot_efficiency()


plot_speedup2()
plot_efficiency2()


