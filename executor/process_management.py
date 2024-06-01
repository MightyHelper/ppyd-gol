import fcntl
import time
from pathlib import Path
from contextlib import contextmanager

# File to track available processes
AVAILABLE_PROCESSES_FILE: Path = Path("available_processes.txt")
FILE_LOCK: Path = Path("available_processes.lock")

@contextmanager
def file_lock(lock_path: Path):
    with open(lock_path, 'w') as lock_file:
        fcntl.flock(lock_file, fcntl.LOCK_EX)
        try:
            yield
        finally:
            fcntl.flock(lock_file, fcntl.LOCK_UN)

def initialize_available_processes() -> None:
    with file_lock(FILE_LOCK):
        if not AVAILABLE_PROCESSES_FILE.exists():
            AVAILABLE_PROCESSES_FILE.write_text('16')  # Set initial number of available cores

def read_available_processes() -> int:
    return int(AVAILABLE_PROCESSES_FILE.read_text().strip())

def update_available_processes(value: int) -> None:
    AVAILABLE_PROCESSES_FILE.write_text(str(value))

def acquire_processes_lock(required_cores: int) -> bool:
    while True:
        with file_lock(FILE_LOCK):
            available_cores = read_available_processes()
            if available_cores >= required_cores:
                update_available_processes(available_cores - required_cores)
                return True
        time.sleep(1)

def release_processes_lock(released_cores: int) -> None:
    with file_lock(FILE_LOCK):
        available_cores = read_available_processes()
        update_available_processes(available_cores + released_cores)

@contextmanager
def processes_lock(required_cores: int) -> bool:
    if acquire_processes_lock(required_cores):
        try:
            yield True
        finally:
            release_processes_lock(required_cores)
    else:
        yield False

# Initialize available processes count
initialize_available_processes()
