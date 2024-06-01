import itertools
import multiprocessing.pool
import subprocess
import tenacity
import yaml
import pandas as pd
import time
import rich.progress as progress
from pathlib import Path
import wandb
import os
os.environ["WANDB_SILENT"] = "true"

configurations = {
    'scaling': {
        "rle": ["mini.rle"],
        "host": ["DGPC"],
        "n": [-1, 1, 4, 9, 16],
        "iw": [10, 100, 500, 1000],
        "ih": [10, 100, 500, 1000],
        "exec_index": list(range(5)),
        "time": [3],
    },
    'length': {
        "rle": [
            "c2-orthogonal.rle",
            "c4-diag-switch-engines.rle",
            "diag-glider.rle",
            "glider.rle",
            "mini.rle"
        ],
        "host": ["DGPC"],
        "n": [4],
        "iw": [100],
        "ih": [100],
        "exec_index": list(range(5)),
        "time": [3]
    }
}

SEQUENTIAL = 'seq'
PARALLEL = 'par'

parallel: Path = Path("..") / "build" / "src" / "main" / "par_gol"
sequential: Path = Path("..") / "build" / "src" / "main" / "gol"
mpirun: Path = Path("/usr/bin/mpirun")
in_data: Path = Path("..") / "data"
out_run: Path = Path("..") / "run"

out_run.mkdir(exist_ok=True)

availableProcesses: multiprocessing.Value = multiprocessing.Value('i', 16)


@tenacity.retry(wait=tenacity.wait_fixed(1), stop=tenacity.stop_after_attempt(3),
                retry=tenacity.retry_if_exception_type(subprocess.TimeoutExpired))
def run(n: int, fil: str, time_ms: int, width: int, height: int):
    # Run process and get output
    if n <= 0:
        process = subprocess.run(
            [sequential, in_data / fil, "t", str(time_ms), str(width), str(height)],
            stdout=subprocess.PIPE,
            text=True,
            timeout=5 + 1.4 * time_ms / 1000
        )
    else:
        process = subprocess.run(
            [mpirun, "-np", str(n), "--oversubscribe", parallel, in_data / fil, "t", str(time_ms), str(width),
             str(height)],
            stdout=subprocess.PIPE,
            text=True,
            timeout=12 + 1.4 * time_ms / 1000
        )
    return process.stdout


def _flatten_dict_gen(d: dict | list, parent_key: str, sep: str = "."):
    if isinstance(d, dict):
        for k, v in d.items():
            new_key = parent_key + sep + k if parent_key else k
            if isinstance(v, dict) or isinstance(v, list):
                yield from flatten_dict(v, new_key, sep=sep).items()
            else:
                yield new_key, v
    elif isinstance(d, list):
        for i, v in enumerate(d):
            new_key = parent_key + sep + str(i) if parent_key else str(i)
            if isinstance(v, dict) or isinstance(v, list):
                yield from flatten_dict(v, new_key, sep=sep).items()
            else:
                yield new_key, v


def flatten_dict(d: dict | list, parent_key: str = '', sep: str = '.'):
    return dict(_flatten_dict_gen(d, parent_key, sep))


def run_item(pair):
    name, item = pair
    global availableProcesses
    (rle, host, n, iw, ih, exec_index, total_time) = item
    file = out_run / f"{name}_{rle}_{host}_{n}_{iw}_{ih}_{exec_index}.yaml"
    if file.exists():
        return
    while True:
        with availableProcesses.get_lock():
            if availableProcesses.value >= abs(n):
                availableProcesses.value -= abs(n)
                break
            # print(f"Need {n} processes, only {availableProcesses.value} available")
        time.sleep(1)
    run_result = None
    start = time.time()
    try:
        run_result = run(n, rle, total_time * 1000, iw, ih)
    except tenacity.RetryError:
        file.unlink(missing_ok=True)
        end = time.time()
        print(f"Failed {name}_{rle}_{host}_{n}_{iw}_{ih}_{exec_index} in {end - start:.2f} seconds")
    if run_result is not None:
        with open(file, "w") as f:
            f.write("---\n")
            f.write(f"index: {exec_index}\n")
            f.write(run_result)
        with open(file, "r") as f:
            yaml_file = yaml.safe_load(f)
        my_run = wandb.init(
            project="gol",
            reinit=True,
            config=yaml_file["run_config"]
        )
        to_dict = flatten_dict({'process': yaml_file["output"]}, sep='.')
        # print(yaml_file)
        my_run.log(to_dict)
        my_run.finish()
    with availableProcesses.get_lock():
        availableProcesses.value += abs(n)


def main():
    global availableProcesses
    for name, configuration in configurations.items():
        experiments = list(itertools.product(*configuration.values()))
        total_time = sum(
            [
                exp[list(configuration.keys()).index("time")]  # * list(configurations.keys()).index("n")
                for exp in experiments
            ]
        )
        print(f"== Total expected time for {name}: {total_time // 60}h {total_time % 60}m ==")
        with multiprocessing.pool.Pool(64) as pool:
            for _ in progress.track(pool.imap_unordered(
                    run_item, [(name, item) for item in experiments]
            ), total=len(experiments)):
                pass
        print(f"== Finished {name} ==")


def srun(n: int, fil: str, time_ms: int, width: int, height: int):
    # Run process and get output
    if n <= 0:
        return [sequential, in_data / fil, "t", time_ms, width, height]
    else:
        return [mpirun, "-np", n, "--oversubscribe", parallel, in_data / fil, "t", time_ms, width, height]


def main2():
    global availableProcesses
    for name, configuration in configurations.items():
        experiments = list(itertools.product(*configuration.values()))
        total_time = sum(
            [
                exp[list(configuration.keys()).index("time")]  # * list(configurations.keys()).index("n")
                for exp in experiments
            ]
        )
        print(f"== Total expected time for {name}: {total_time // 60}h {total_time % 60}m ==")
        for rle, host, n, iw, ih, exec_index, total_time in experiments:
            with open('/tmp/batch.sh', 'w') as f:
                f.write(f"""#!/bin/bash
                ##SBATCH --job-name={name}_{rle}_{host}_{n}_{iw}_{ih}_{exec_index}
                {" ".join([str(x) for x in srun(n, rle, total_time * 1000, iw, ih)])}
                """)
            subprocess.run(["sbatch", "/home/admin/batch.sh"])
        print(f"== Finished {name} ==")


main()
