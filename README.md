# ppyd-gol
A basic implementation of the Game of Life using MPI.

Cmake 3.28 is required - you need the ppa (https://apt.kitware.com/)

## Compilation

To compile the project, please run the following commands:
```bash
mkdir build
cd build
cmake ..
make -j 32
```

## Execution
From the `build` directory, you can run the following commands:
```bash
src/main/gol
```

```bash
mpirun -n 9 src/main/par_gol
```

## Debugging

### GDB

GDB is a powerful debugger that integrates well with **CLion**. You can run it by debugging a CMake run configuration in **CLion**.

Alternatively, you can run it from the command line (from the `build` directory):
```bash
$ gdb src/main/gol
(gdb) run
...
(gdb) exit
```


### Valgrind

Valgrind is a really cool tool for detecting memory leaks and other reasons your program might crash.

Valgrind is also available in **CLion** as another option alongside debugging, but you can run it from the command line as well.

#### Sequential

```bash
valgrind src/main/gol
```

#### Parallel

When running with MPI, Valgrind may produce some MPI-related warnings that can be ignored:

```bash
mpirun -n 9 valgrind src/main/par_gol
```


## Experiment execution

1. Create a WANDB account wandb.ai
2. Install WANDB locally using pip
3. Run `wandb login` and paste in your token
4. Create a sweep.yaml
5. Run `wandb sweep -p <project> -e <entity>` to create the sweep
6. Create a `available_processes.txt` inside `executor` with the number of cores available on the machine
7. Run `nohup <sweep agent cmd> &` as many times as required from `executor`


