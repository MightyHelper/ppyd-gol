import pandas as pd
import json
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

df = pd.read_csv("data/project.csv").sort_index(axis=1)
df['config.ow'] = df['config.iw'] * np.abs(df['config.n'])
df['config.oh'] = df['config.ih'] * np.abs(df['config.n'])
df['config.is'] = df['config.iw'] * df['config.ih']
df['config.os'] = df['config.ow'] * df['config.oh']
scaling_sweep = df['sweep_id'] == 'scaling'
parallel = df['config.n'] > 0
scaling_df = df[scaling_sweep]
scaling_parallel_df = scaling_df
scaling_parallel_results = scaling_parallel_df.drop(columns=['config.rle', 'name', 'sweep_id', 'config.host', 'summary._wandb']).groupby(['config.os', 'config.n']).mean()
scaling_parallel_results['ms/its'] = scaling_parallel_results['summary.process.0.results.time.total'] / scaling_parallel_results['summary.process.0.results.iterations']

par = scaling_parallel_results.reset_index()
n_values = set(par['config.n'])
palette = sns.color_palette("husl", len(n_values))

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
    return par[items]['ms/its'].values[0]

def speedup(o_size, n):
    return time_per_it(o_size, -1) / time_per_it(o_size, n)

def efficiency(o_size, n):
    return speedup(o_size, n) / n

def plot_speedup():
    fig, ax = plt.subplots()
    for i, x in enumerate(n_values):
        ax.plot(par['config.os'], [speedup(o_size, x) for o_size in par['config.os']], color=palette[i], label=f'{x}' if x > 0 else 'Sequential')
    ax.legend(title='Nodes')
    ax.set_xlabel('Outer Size')
    ax.set_ylabel('Speedup')
    ax.set_xscale('log')
    plt.savefig("data/speedup_in_os.png")

def plot_efficiency():
    fig, ax = plt.subplots()
    for i, x in enumerate(n_values):
        ax.plot(par['config.os'], [efficiency(o_size, x) for o_size in par['config.os']], color=palette[i], label=f'{x}' if x > 0 else 'Sequential')
    ax.legend(title='Nodes')
    ax.set_xlabel('Outer Size')
    ax.set_ylabel('Efficiency')
    ax.set_xscale('log')
    plt.savefig("data/efficiency_in_os.png")

plot_speedup()
plot_efficiency()

