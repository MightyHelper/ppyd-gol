import pandas as pd
import json
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns

df = pd.read_csv("data/project.csv").sort_index(axis=1)
df['config.ow'] = df['config.iw'] * np.abs(df['config.n'])
df['config.oh'] = df['config.ih'] * np.abs(df['config.n'])
df['config.is'] = df['config.iw'] * df['config.ih']
df['config.os'] = df['config.ow'] * df['config.oh']
scaling_sweep = df['sweep_id'] == 'file'
merged = df[scaling_sweep].drop(columns=['name', 'sweep_id', 'config.host', 'summary._wandb']).groupby(
    ['config.rle', 'config.n']).mean().reset_index()
merged2 = df[scaling_sweep].drop(columns=['name', 'sweep_id', 'config.host', 'summary._wandb']).groupby(
    ['config.rle', 'config.n']).std().reset_index()

merged['ms/its'] = merged['summary.process.0.results.time.total'] / merged['summary.process.0.results.iterations']
merged2['ms/its'] = merged2['summary.process.0.results.time.total'] / merged2['summary.process.0.results.iterations']


# fig, ax = plt.subplots()
# n_values = list(set(merged['config.n']))
# palette = sns.color_palette("husl", len(n_values))
# for x in n_values:
#     me = merged[merged['config.n'] == x]
#     mini_rle = me[me['config.rle'] == 'mini.rle']
#     xv = me['summary.process.0.results.time.total'] / mini_rle.iloc[0]['summary.process.0.results.time.total'] * 100
#     print(xv)
#     plt.bar(
#         x=me['config.rle'],
#         height=xv,
#         color=palette[n_values.index(x)],
#         label=f'{x}' if x > 0 else 'Sequential',
#     )
# ax.set_xlabel('Total time')
# ax.set_ylabel('Input File')
# # Set ylim
# plt.ylim(99.995, 100.005)
# # Rotate x-axis labels
# plt.xticks(rotation=45)
# # move the legend
# ax.legend(title='Nodes', loc='upper left', bbox_to_anchor=(1, 1))
# # tichg
# plt.tight_layout()
# plt.show()

# Create figure and axis
fig, ax = plt.subplots()
n_values = sorted(set(merged['config.n']))
palette = sns.color_palette("husl", len(n_values))

# Unique rle values
rle_values = merged['config.rle'].unique()

# Set bar width and positions
bar_width = 0.2
bar_positions = range(len(rle_values))

# Loop through each unique 'config.n' value
for i, x in enumerate(n_values):
    me = merged[merged['config.n'] == x]
    mini_rle = me[me['config.rle'] == 'mini.rle']
    xv = me['ms/its'] / mini_rle.iloc[0]['ms/its']
    # Adjust the bar position
    bar_offset = [pos + i * bar_width for pos in bar_positions]
    ax.bar(
        bar_offset,
        height=1-xv,
        bottom=1,
        width=bar_width,
        color=palette[i],
        # add error bars
        yerr=merged2[merged2['config.n'] == x]['ms/its'],
        label=f'{x}' if x > 0 else 'Sequential'
    )

# add hline
ax.axhline(y=1, color='black', linestyle='--')

# Set x-ticks positions and labels
ax.set_xticks([pos + (len(n_values) - 1) * bar_width / 2 for pos in bar_positions])
ax.set_xticklabels(rle_values)

# Set labels and title
ax.set_xlabel('Input File')
ax.set_ylabel('Relative time/iterations to mini.rle')
# ax.set_ylim(0.9, 1.1)
# set y-ticks
# ax.set_yticks(np.linspace(0.9, 1.1, 5))
# set y-ticks labels without scientific notation
ax.get_yaxis().get_major_formatter().set_useOffset(False)
plt.xticks(rotation=45)
ax.legend(title='Nodes', loc='upper left', bbox_to_anchor=(1, 1))
plt.tight_layout()

# save the plot
plt.savefig("data/file.png")
