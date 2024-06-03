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

# Performa  linear regression on each group of par

par = scaling_parallel_results.reset_index()
n_values = set(par['config.n'])
palette = sns.color_palette("husl", len(n_values))

regression = {}

for i, x in enumerate(n_values):
    subset = par[par['config.n'] == x]
    x = subset['config.os']
    y = subset['ms/its']
    m, b = np.polyfit(x, y, 1)
    print(f"Linear in OS: {m}x + {b}")
    rmse = np.sqrt(np.mean((y - (m * x + b))**2))
    print(f"RMSE: {rmse}")
    regression[i] = (m, b)

def extrapolate(n, os):
    m, b = regression[list(n_values).index(n)]
    os_b = m * os + b
    print(f"Extrapolating for n={n} and os={os}: {os_b}")
    return os_b

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

## Plot regression lines
fig, ax = plt.subplots()
for i, x in enumerate(n_values):
    rang = np.linspace(0, 1000000, 1000)
    ax.plot(rang, extrapolate(x, rang), color=palette[i], label=f'{x}' if x > 0 else 'Sequential')
    subset = par[par['config.n'] == x]
    ax.scatter(subset['config.os'], subset['ms/its'], color=palette[i])
ax.legend(title='Nodes')
ax.set_xlabel('Outer Size')
ax.set_ylabel('Milliseconds per iteration')
plt.savefig("data/linear_in_os_regression.png")


# X: Nodes
# Y: speedup computed based on linear regression

fig, ax = plt.subplots()
speedup = {}
o_size = 100000
t_i_seq = extrapolate(-1, o_size)

for i, x in enumerate(n_values):
    if x == -1:
        continue
    subset = par[par['config.n'] == x]
    t_i_par = extrapolate(x, o_size)
    speedup[x] = t_i_seq / t_i_par
ax.plot(speedup.keys(), speedup.values())
ax.scatter(speedup.keys(), speedup.values())
# Add linear speedup
ax.plot(speedup.keys(), speedup.keys())
ax.legend(['Speedup', 'Linear'])
ax.set_xlabel('Nodes')
ax.set_ylabel('Speedup')
fig.savefig("data/speedup.png")




