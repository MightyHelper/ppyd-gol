import pandas as pd 
import wandb
from rich import progress
api = wandb.Api()

runs = api.runs("rainbow-talon/gol")
items = []
for run in progress.track(runs, total=len(runs), transient=True):
    items.append({
        **{f"summary.{k}": v for k, v in dict(run.summary).items()},
        **{f"config.{k}": v for k,v in run.config.items()},
        "name": run.name,
        "sweep_id": run.sweep.name
    })


runs_df = pd.DataFrame(items)
runs_df.to_csv("data/project.csv", index=False)
