import itertools
import multiprocessing.pool
import subprocess
import tenacity
import time
import rich.progress as progress
from pathlib import Path
from threading import Lock

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
            "golly-ticker.rle",
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

availableProcesses = 16
lock = Lock()

@tenacity.retry(wait=tenacity.wait_fixed(5), stop=tenacity.stop_after_attempt(3),
                retry=tenacity.retry_if_exception_type(subprocess.TimeoutExpired))
def run(n: int, fil: str, time_ms: int, width: int, height: int) -> str | None:
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
            [mpirun, "-np", str(n), "--oversubscribe", parallel, in_data / fil, "t", str(time_ms), str(width), str(height)],
            stdout=subprocess.PIPE,
            text=True,
            timeout=12 + 1.4 * time_ms / 1000
        )
    return process.stdout


def run_item(pair):
    name, item = pair
    global availableProcesses, lock
    (rle, host, n, iw, ih, exec_index, total_time) = item
    file = out_run / f"{name}_{rle}_{host}_{n}_{iw}_{ih}_{exec_index}.txt"
    if file.exists():
        return
    with lock:
        availableProcesses -= n
    run_result = None
    try:
        run_result = run(n, rle, total_time * 1000, iw, ih)
    except tenacity.RetryError:
        file.unlink(missing_ok=True)
        print(f"Failed {name}_{rle}_{host}_{n}_{iw}_{ih}_{exec_index}")
    if run_result is not None:
        with open(file, "w") as f:
            f.write("---\n")
            f.write(f"index: {exec_index}\n")
            f.write(run_result)
    with lock:
        availableProcesses += n


def main():
    for name, configuration in configurations.items():
        experiments = list(itertools.product(*configuration.values()))
        total_time = sum(
            [
                exp[list(configuration.keys()).index("time")]  # * list(configurations.keys()).index("n")
                for exp in experiments
            ]
        )
        print(f"== Total expected time for {name}: {total_time // 60}h {total_time % 60}m ==")
        with multiprocessing.pool.Pool() as pool:
            for _ in progress.track(pool.imap_unordered(
                    run_item, [(name, item) for item in experiments]
            ), total=len(experiments)):
                with lock:
                    print(availableProcesses)
        print(f"== Finished {name} ==")

main()
