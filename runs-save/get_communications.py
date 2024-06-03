import pandas as pd
import json
import matplotlib.pyplot as plt
import numpy as np
df = pd.read_csv("data/project.csv").sort_index(axis=1)
df['config.ow'] = df['config.iw'] * np.abs(df['config.n'])
df['config.oh'] = df['config.ih'] * np.abs(df['config.n'])
df['config.is'] = df['config.iw'] * df['config.ih']
df['config.os'] = df['config.ow'] * df['config.oh']
scaling_sweep = df['sweep_id'] == 'scaling'
parallel = df['config.n'] > 0
scaling_df = df[scaling_sweep]
scaling_parallel_df = scaling_df[scaling_df['config.n'] > 0]
scaling_parallel_results = scaling_parallel_df.drop(columns=['config.rle', 'name', 'sweep_id', 'config.host', 'summary._wandb']).groupby(['config.n']).mean()
scaling_parallel_results['its/ms'] = scaling_parallel_results['summary.process.0.results.iterations'] / scaling_parallel_results['summary.process.0.results.time.total']
import matplotlib.pyplot as plt
import pandas as pd

# Create stacked bar chart
fig, ax = plt.subplots()

# Define colors
colors = ['#ff2222', '#66b3ff', '#99ff99', '#ffcc99']

scaling_parallel_results['summary.process.0.results.time.unknown'] = scaling_parallel_results[
                                                                         'summary.process.0.results.time.total'] - (
                                                                                 scaling_parallel_results[
                                                                                     'summary.process.0.results.time.communication'] +
                                                                                 scaling_parallel_results[
                                                                                     'summary.process.0.results.time.idle'] +
                                                                                 scaling_parallel_results[
                                                                                     'summary.process.0.results.time.compute'])
scaling_parallel_results['summary.process.0.results.time.communication_norm'] = scaling_parallel_results[
                                                                                    'summary.process.0.results.time.communication'] / \
                                                                                scaling_parallel_results[
                                                                                    'summary.process.0.results.time.total']
scaling_parallel_results['summary.process.0.results.time.compute_norm'] = scaling_parallel_results[
                                                                              'summary.process.0.results.time.compute'] / \
                                                                          scaling_parallel_results[
                                                                              'summary.process.0.results.time.total']
scaling_parallel_results['summary.process.0.results.time.idle_norm'] = scaling_parallel_results[
                                                                           'summary.process.0.results.time.idle'] / \
                                                                       scaling_parallel_results[
                                                                           'summary.process.0.results.time.total']
scaling_parallel_results['summary.process.0.results.time.unknown_norm'] = scaling_parallel_results[
                                                                              'summary.process.0.results.time.unknown'] / \
                                                                          scaling_parallel_results[
                                                                              'summary.process.0.results.time.total']
# Plot each component
(scaling_parallel_results[[
    'summary.process.0.results.time.communication_norm',
    'summary.process.0.results.time.idle_norm',
    'summary.process.0.results.time.compute_norm',
    'summary.process.0.results.time.unknown_norm'
]]).plot(kind='bar', stacked=True, color=colors, ax=ax)

# Add total line
# ax.plot(scaling_parallel_results.index, scaling_parallel_results['summary.process.0.results.time.total'], color='black', marker='o', label='Total')

# Set labels and title
ax.set_xlabel('Process Count (n)')
ax.set_ylabel('Percent')

# Add legend
ax.legend(['Communication', 'Idle', 'Compute', 'Unknown'], title='Reference', loc='upper left',
          bbox_to_anchor=(1, 1))
# tight layout
plt.tight_layout()
plt.savefig("data/communication.png")
