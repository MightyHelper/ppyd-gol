import pandas as pd 
import wandb
from rich import progress
import multiprocessing.pool as pool
api = wandb.Api()

runs = api.runs("rainbow-talon/gol")
items = []
def process_run(run):
    print(f"Getting run {run.name}")
    history = run.history()
    run_data = {
        **{f"summary.{k}": v for k, v in dict(run.summary).items()},
        **{f"config.{k}": v for k, v in run.config.items()},
        **(dict(history.iloc[0]) if run.state == 'finished' and len(history) > 0 else {}),
        "name": run.name,
        "sweep_id": run.sweep.name
    }
    print(f"Got run {run.name}")
    return run_data

local_runs = list(progress.track(runs, total=len(runs), transient=True))
print("Downloaded!")
# with pool.ThreadPool() as pool:
for run in progress.track(local_runs, total=len(local_runs), transient=True):
    items.append(process_run(run))



runs_df = pd.DataFrame(items)
runs_df.to_csv("data/project.csv", index=False)
