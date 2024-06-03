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
scaling_sweep = (df['sweep_id'] == 'scaling') | (df['sweep_id'] == 'scaling2')
parallel = df['config.n'] > 0
scaling_df = df[scaling_sweep]
scaling_parallel_df = scaling_df
scaling_parallel_results = scaling_parallel_df.drop(columns=['config.rle', 'name', 'config.host', 'summary._wandb']).groupby(['config.os', 'config.n', 'sweep_id']).mean()
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
    result = par[items]['ms/its'].values[0]
    print(f"Time per iteration for outer size {o_size} and {n} nodes: {result}")
    return result

def speedup(o_size, n):
    s = time_per_it(o_size, -1) / time_per_it(o_size, n)
    print(f"Speedup for outer size {o_size} and {n} nodes: {s}")
    return s

def efficiency(o_size, n):
    e = speedup(o_size, n) / n
    print(f"Efficiency for outer size {o_size} and {n} nodes: {e}")
    return e

def plot_speedup():
    fig, ax = plt.subplots()
    for i, x in enumerate(n_values):
        if x < 0:
            continue
        os_values = list(sorted(set(par[par['config.n'] == x]['config.os']).intersection(set(par[par['config.n'] == -1]['config.os']))))
        print(x, os_values)
        ax.plot(os_values, [speedup(o_size, x) for o_size in os_values], color=palette[i], label=f'{x}' if x > 0 else 'Sequential')
    ax.legend(title='Nodes')
    ax.set_xlabel('Outer Size')
    ax.set_ylabel('Speedup')
    plt.savefig("data/speedup_in_os.png")

def plot_efficiency():
    fig, ax = plt.subplots()
    for i, x in enumerate(n_values):
        if x < 0:
            continue
        os_values = list(sorted(set(par[par['config.n'] == x]['config.os']).intersection(set(par[par['config.n'] == -1]['config.os']))))
        print(x, os_values)
        ax.plot(os_values, [efficiency(o_size, x) for o_size in os_values], color=palette[i], label=f'{x}' if x > 0 else 'Sequential')
    ax.legend(title='Nodes')
    ax.set_xlabel('Outer Size')
    ax.set_ylabel('Efficiency')
    plt.savefig("data/efficiency_in_os.png")

plot_speedup()
plot_efficiency()


