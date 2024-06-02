import subprocess
import tenacity
import yaml
import wandb
from pathlib import Path
import time
from typing import Optional
from process_management import processes_lock
from utils import flatten_dict

SEQUENTIAL: str = 'seq'
PARALLEL: str = 'par'
parallel: Path = Path("..") / "build" / "src" / "main" / "par_gol"
sequential: Path = Path("..") / "build" / "src" / "main" / "gol"
mpirun: Path = Path("/usr/bin/mpirun")
in_data: Path = Path("..") / "data"
out_run: Path = Path("..") / "run"
out_run.mkdir(exist_ok=True)

@tenacity.retry(wait=tenacity.wait_fixed(1), stop=tenacity.stop_after_attempt(3),
                retry=tenacity.retry_if_exception_type(subprocess.TimeoutExpired))
def run(n: int, fil: str, time_ms: int, width: int, height: int) -> str:
    if n <= 0:
        process = subprocess.run(
            [sequential, in_data / fil, "t", str(time_ms), str(width), str(height)],
            stdout=subprocess.PIPE,
            text=True,
            timeout=60 + 1.4 * time_ms / 1000
        )
    else:
        process = subprocess.run(
            [mpirun, "-np", str(n), "--oversubscribe", parallel, in_data / fil, "t", str(time_ms), str(width),
             str(height)],
            stdout=subprocess.PIPE,
            text=True,
            timeout=120 + 1.4 * time_ms / 1000
        )
    return process.stdout

def run_item(config: dict) -> None:
    rle: str = config['rle']
    host: str = config['host']
    n: int = config['n']
    iw: int = config['iw']
    ih: int = config['ih']
    exec_index: int = config['exec_index']
    total_time: int = config['time']

    file: Path = out_run / f"{rle}_{host}_{n}_{iw}_{ih}_{exec_index}.yaml"
    if file.exists():
        return

    with processes_lock(abs(n)) as l:
        if not l:
            return # Could not acquire lock

        run_result: Optional[str] = None
        start: float = time.time()
        try:
            run_result = run(n, rle, total_time * 1000, iw, ih)
        except tenacity.RetryError:
            file.unlink(missing_ok=True)
            end = time.time()
            print(f"Failed {file.name} in {end - start:.2f} seconds")

        if run_result is not None:
            file.write_text(f"---\nindex: {exec_index}\n{run_result}")
            with file.open("r") as f:
                yaml_file = yaml.safe_load(f)
            # my_run = wandb.init(
                #project="gol",
                #reinit=True,
                #config=yaml_file["run_config"]
            #)
            to_dict: dict = flatten_dict({'process': yaml_file["output"]}, sep='.')
            wandb.log(to_dict)
            # my_run.finish()
