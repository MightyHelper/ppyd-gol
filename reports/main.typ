#import "algo.typ": gol, gol-par
#import "@preview/codelst:2.0.1": sourcecode


#set document(
  title: "Parallel Implementation of Conway's Game of Life",
  author: "Micaela Del Longo, Federico Williamson",
)

#set page(
  margin: (top: 5cm, bottom: 4cm, left: 4cm, right: 4cm),
  numbering: "1/1"
)

#set text(
  font: "Times New Roman",
  size: 10pt,
  region: "GB"
)

#set heading(numbering: "1.1.")
#show heading: it => {
  it
  v(0.6em)  // el espacio default de typst
}
#show heading.where(level: 1): set text(size: 14pt, weight: "bold")
#show heading.where(level: 2): set text(size: 12pt, weight: "bold")
#show heading.where(level: 3): set text(size: 10pt, weight: "bold")
#show heading.where(level: 4): set text(size: 10pt, style: "italic")

#set par(
  justify: true,
)

#set bibliography(
  title: none,
  // style: "apa"
  style: "cite.csl"
)

#let to-verify(content) = [
  #text(fill:red, content)
  #text(fill:red, weight: "bold")[\- To be reviewed]
]

#let to-do(content) = [
  #text(fill:purple, weight: "bold")[TODO: ]
  #text(fill: purple, content)
]

#let university(name) = [
  #text(name)
]

#let title(content) = {
  text(weight: "bold", 
  size: 16pt, 
  content)
}

#let count(txt) = [
  #txt
  #if false [#str(txt).split(" ").len() words] else []
]
#set math.equation(supplement: "Formula")

// end settings ----------------------------------------------------------------------
#title[Parallel Implementation of Conway's Game of Life]

#text(size: 12pt, weight: "bold")[Micaela Del Longo, Federico Williamson]

#university[Programación Paralela y Distribuida, Licenciatura en Ciencias de la Compuatción, Facultad de Ingeniería, Universidad Nacional de Cuyo]
// la profe pide esto https://aulaabierta.ingenieria.uncuyo.edu.ar/pluginfile.php/161671/mod_resource/content/4/RequisitosPresentacionInforme_2023.pdf

#text(style: "italic")[
  *Abstract:* #count("This work proposes a parallel implementation of Conway's Game of Life, a classic cellular automaton, using the Message Passing Interface (MPI). This approach aims to exploit the inherent parallelism of the game to efficiently distribute computation across multiple processes, thereby harnessing the power of modern parallel computing architectures.")
  
  *Keywords:* Conway's Game of Life, MPI, Domain decomposition, Master-Worker algorithm.
]

#pagebreak()

#outline(indent: true)

/* 
Entrega 1: 
- Diseñar e implementar la versión secuencial. 
- Analizar el algoritmo elegido e identificar las posibles porciones de código o datos que podrían ser factibles de paralelizarse. 
- Realizar la entrega correspondiente al análisis realizado, y la versión secuencial, a través del Aula Abierta. 
*/ 

#pagebreak()

= Introduction <intro>
/*
1. This should contain the background of the problem, [DONE]
2. why it is important, [DONE]
3. and what others have done to solve this problem. All related existing work should be properly described and referenced. 
4. The proposed solution should be briefly described, with explanations of how it is different from, and superior to, existing solutions. 
5. The last paragraph should be a summary of what will be described in each subsequent section of the paper.
*/

Conway's Game of Life @gardner1970mathematical is a cellular automaton devised by the British mathematician John Horton Conway in 1970. It is a zero-player game, where its evolution is determined by its initial state, needing no further input from a player. One interacts with the Game of Life by creating an initial configuration and observing how it evolves. 

Game of Life's applicability spans diverse fields, including biology, physics, computer science, and beyond. Studying the dynamics of complex systems through it offers insights into pattern formation, self-organisation, and emergent phenomena, with potential implications for designing efficient algorithms, modelling biological processes, and simulating natural systems. Thus, the importance of understanding and efficiently simulating Conway's Game of Life extends beyond recreational curiosity.

Regarding parallel implementations of Game of Life, in some @GiorgospanGOL @PanPapagGOL domain decomposition is a common strategy employed to divide the computational workload among multiple processing units. This approach involves breaking the grid representing the cellular automaton into smaller subdomains, each assigned to a different processing unit for independent computation.

Technology wise, OpenMP (Open Multi-Processing), a widely used API for shared memory multiprocessing in C, C++, and Fortran, is often employed for parallelisation. Developers annotate their code with OpenMP directives to indicate regions that can be executed concurrently.

The subsequent sections of the paper delve into various aspects of the parallel implementation of Game of Life.
In @GoL, a brief explanation of the game's rules and mechanics will be provided.
Following this, @pseudocode offers a step-by-step breakdown of the algorithm's key components and operations.
The subsequent @parallel-analysis will explore strategies for parallelising said algorithm.
@parallel-impl describes the parallel design and implementation of Game of Life. The tests conducted on the implementations are detailed in @exp-design, its results and their analysis are discussed in @performance. Finally, in @conclusion a conclusion is provide.
In @appendix, detailed information regarding the initial state representation (@appendix-1) and instructions for compiling and executing the source code (@appendix-2) will be provided.

/* #to-verify[
  To evaluate the performance of our MPI implementation, a series of experiments on various parallel computing platforms, ranging from multicore CPUs to distributed memory clusters, were conducted. Scalability, load balancing, and overhead characteristics under different problem sizes and MPI configurations were analysed. The results demonstrate significant speed-up compared to the sequential version, highlighting the effectiveness of MPI in harnessing parallelism in Conway's Game of Life.
] */

#pagebreak()

= Game of Life <GoL>

Conway's Game of Life is a cellular automaton that operates on a grid of cells, each of which can be in one of two states: _alive_, represented by a white cell, or _dead_, represented by a black cell. At each time step, the transitions rules displayed in @rules, herein explained, are applied:  //@mica me hace un poco de ruido pero lo uniste muy bien (el comment es más para mí por si se me ocurre algo)

+ *Birth*: A dead cell with exactly three live neighbours#footnote[A neighbour is defined as any adjacent cell, including diagonals. See @neighbours] becomes alive (is "born") in the next generation.

+ *Survival*: A live cell with two or three live neighbours remains alive in the next generation.

+ *Death by Isolation*: A live cell with fewer than two live neighbours dies due to under-population in the next generation.

+ *Death by Overcrowding*: A live cell with more than three live neighbours dies due to overcrowding in the next generation, as if by lack of resources.

These rules are applied simultaneously to every cell in the grid for each generation. The game progresses in discrete time steps, with each step creating a new configuration of live and dead cells based on the current state of the grid. The resulting patterns can exhibit a wide range of behaviours, including static patterns, oscillations, and complex evolving structures.

#figure(
  image("images/neighbours.png"),
  caption: [A dead cell (black block in centre with red border) surrounded by its eight live neighbours (white blocks numbered one to eight).],
) <neighbours>

#figure(
  image("images/Rules.png"),
  caption: [Graphical representation of transition rules. Each column shows the application of *one* rule over *the centre* cell only (marked with a red border), ignoring its effect over the other surrounding cells.]
) <rules>

// #to-do[agregar otra imagen con todos los efectos. Si hay tiempo lo hacemos]

= Sequential Pseudocode <pseudocode>

To provide a clear understanding of the sequential implementation of Conway's Game of Life, the pseudocode detailing the algorithm's key steps is presented in @gol-algoritm. It outlines the sequential evolution of the game grid from one generation to the next.

In the algorithm, the evolve function takes the current grid state as input and returns the grid state after one generation. It iterates over each cell in the grid, counts the number of live neighbours for each cell using the _countLiveNeighbours_ function, and applies the rules of the Game of Life to determine the state of each cell in the next generation. The resulting grid represents the next generation of the game. 

This sequential algorithm forms the basis for parallelisation using MPI, where the grid is divided among multiple processes to enable concurrent computation and the game evolution.

#gol <gol-algoritm>

= Parallelisation Analysis <parallel-analysis>

This section delves into the strategies and considerations involved in parallelising Conway's Game of Life. It explores various aspects of decomposition strategies (@decomposition), algorithm models (@algo-model), communication models (@comm-model), and other optimisation opportunities (@other-opti) aimed at enhancing the efficiency and scalability of the parallel implementation.

== Decomposition Strategy <decomposition>

As discussed in the introduction, this paper proposes to apply domain decomposition. This strategy has been explored in several studies @PengshanGOL with various styles such as "Grid domain decomposition", "Horizontally striped domain decomposition" and "Vertically striped domain decomposition".

// The choice to utilize domain decomposition over other strategies is driven by its effectiveness in parallelizing computations in cellular automata like Conway's Game of Life. Domain decomposition allows for the workload to be divided among multiple processing units, each handling a specific subdomain of the overall grid. This division facilitates concurrent computation and reduces the complexity of managing dependencies between cells across different regions of the grid.

Domain decomposition is a natural choice because cells are only 
related to directly adjacent cells. This means that they only need to know about neighbouring locations to compute their next state.

This makes domain decomposition almost trivial. Since it is only required a ghost layer#footnote[To be able to compute the short-range interactions, MPI processes need not only access to the data of cells they "own" but also information about cells from neighbouring subdomains, referred to as _"ghost"_ atoms @LammpsCommunication.] of adjacent neighbours every time step for each node. All internal cells can be computed with no extra information.

#figure(
  box(
    image("images/ghost-cells.png", width: 100%),
    inset: (bottom:-15%, top:-10%),
    clip: true,
    width: 70%,
  ),
  caption: [Cells distributed among processes. In light grey are shown the ghost cells and in red, the internal cells.]) <ghost>

The implementation of these strategies is not trivial, as it requires that each node (i) is aware of its local grid partition border and (ii) communicates with its neighbouring nodes in order to exchange boundary information. This exchange is crucial for ensuring that each node has the necessary data to compute the next generation correctly.

In this work, grid domain decomposition will be the primary method applied. If time permits Horizontally striped and vertically striped domain decomposition will also be implemented for comparative analysis.

This is due to grid domain decomposition being more promising when it comes to scaling. As the number of cells computed by a process increases, this strategy ensures that the perimeter stays lower compared to the area.

The following subsections provide further explanation on these strategies.

=== Grid Domain Decomposition <grid-decomposition>

In grid domain decomposition, the grid is divided into smaller sub-grids, and each sub-grid is assigned to a different process. This approach is straightforward and ensures that each process operates on a contiguous portion of the grid. However, if optimisations are applied, such that the computation is focused only where live cells are, it may lead to load imbalance if some regions of the grid contain more live cells than others.

=== Horizontally Striped Domain Decomposition <horizontal-decomposition>

Horizontally striped domain decomposition involves dividing the grid into horizontal strips, with each strip assigned to a different process. This approach can help mitigate load imbalance by ensuring that each process handles roughly the same number of rows. However, it may not be suitable for grids where certain patterns or structures span multiple rows.

=== Vertically Striped Domain Decomposition <vert-decomposition>

Vertically striped domain decomposition divides the grid into vertical strips, assigning each strip to a different process. Similar to horizontally striped decomposition, this method aims to balance the workload across processes by distributing columns evenly. But, it may face challenges with load imbalance if certain columns contain more live cells than others.

#figure(
  grid(
    columns: 3,
    rows: 2,
    column-gutter: 6%,
    row-gutter: 1%,
    image("images/Grid_domain_decomposition.png", width: 100%),
    image("images/Horizontally_striped_domain_decomposition.png", width: 100%),
    image("images/Vertically_striped_domain_decomposition.png", width: 100%),
    align(center)[(a)],
    align(center)[(b)],
    align(center)[(c)],
  ),
  kind: image,
  caption: [
    (a) Grid domain decomposition.
    (b) Horizontally striped domain decomposition.
    (c) Vertically striped domain decomposition.
  ]
) <grid-styles>

== Parallel Algorithm Model <algo-model>

Among all the possible parallel algorithm models, the master-worker model is considered the most applicable to this work. In this model, the master process is responsible for distributing tasks to several worker processes, which perform the actual computations. The master also handles the aggregation of results from the workers. 

This approach is well-suited for Game of Life, where the grid can be divided into subdomains, and each worker can process a subdomain independently. The master-worker model ensures efficient task distribution and management, making it an ideal choice for handling the parallelisation of cellular automaton simulations.

While the master-worker model offers an effective framework, it's important to consider the nuances of task assignment within this model. Aside from certain load balancing strategies that could offer advantages if implemented dynamically, dynamic assignment would not provide any further performance improvements due to the use of a large grain size#footnote[In this context, large grain size refers to the significant amount of work assigned to each process (a whole sub-region of the domain).].

When the tasks are large, the overhead of dynamically assigning tasks can outweigh the benefits, as the processes spend more time managing the task distribution rather than performing computations#footnote[Since the tasks would likely take a while and not change between processes.]. Static assignment, where tasks are predetermined and assigned at the start, can be more efficient in such scenarios because it minimises the overhead and maximises the computational throughput. Therefore, while dynamic load balancing strategies can be beneficial in some situations, they are not advantageous for this specific work due to its large grain size.

== Communication Model <comm-model>

In this implementation of Conway's Game of Life, the decision was made to employ the message passing communication model, specifically, utilising the Message Passing Interface (MPI) protocol.

MPI provides a standardised and widely-supported framework for communication in parallel and distributed computing environments. This standardisation ensures portability across different architectures and platforms, facilitating efficient deployment on various parallel computing systems.

Message passing enables efficient communication and data exchange between processes. It allows processes to explicitly send and receive data, promoting seamless coordination and synchronisation between distributed entities.

== Other Optimisation Opportunities <other-opti>

In addition to the strategies discussed in the previous sections, there are other optimisation opportunities available, such as load balancing. Two styles of load balancing explored in this work are: Recursive Coordinate Bisection (RCB) and grid-based methods @LammpsBalance.

RCB is a "tiling" method which does not produce a logical regular grid of processors. Rather, it tiles the simulation domain with irregular rectangular sub-boxes of varying size and shape to have equal numbers of particles (or weight) in each sub-box, as in the following rightmost diagram (@load-balance).

#figure(
  grid(
    columns: 3,
    rows: 2,
    column-gutter: 2%,
    row-gutter: 1%,
    image("images/balance_uniform.jpg", width: 100%),
    image("images/balance_nonuniform.jpg", width: 100%),
    image("images/balance_rcb.jpg", width: 100%),
    align(center)[(a)],
    align(center)[(b)],
    align(center)[(c)],
  ),
  kind: image,
  caption: [
    (a) No load balance strategy.
    (b) Grid method for load balancing.
    (c) RCB method for load balancing.
  ]
) <load-balance>

Grid balancing, on the other hand, involves organising computational resources in a structured grid-like manner to distribute the workload evenly across the system. Unlike RCB (Recursive Coordinate Bisection), which employs irregular sub-boxes, grid balancing typically involves dividing the simulation domain into a logical grid of processors.

Currently, there are no plans of implementing load balancing. However, if during optimisation it is found that processing can be restricted to areas with live cells (e.g., using a quadtree), then load balancing may be considered.

= Parallel Design and Implementation <parallel-impl>

/*
Entrega 2:
Descripción de las estrategias y decisiones de diseño consideradas para el desarrollo de la versión paralelo/distribuida, a partir de la secuencial. Hacer énfasis especialmente en las posibles diferencias que se hayan podido suscitar en el análisis realizado e informado en el inciso a). 

Pseudocódigo secuencial y pseudocódigo paralelo/distribuido propuesto.

Código fuente de la versión paralelo/distribuida. Si fuese el caso, actualizar también la versión secuencial (sólo si hubo cambios desde la entrega anterior).
*/

As detailed in the previous sections, the parallel implementation employs grid domain decomposition. Specifically, the chosen algorithm model is the master-worker model. Furthermore, MPI (Message Passing Interface) was selected for communication, using a message-passing approach. At this stage, however, no load balancing strategy has been applied.

The parallel algorithm does not significantly differ from the sequential implementation. @gol-par shows how to apply the sequential implementation across multiple processes to effectively simulate a larger domain.

#gol-par <gol-par>

== Design

Most of the code from the sequential version was recycled and used here. However, there were significant changes to accommodate the parallel implementation:

- All simulation cells are two cells wider and two cells taller than required in order to accommodate for a single ghost cell layer on each side.
- A global coordinate system was instated to allow common referencing of the same cell.
- Communication between processes was developed in the following manner:
  - First, all processes asynchronously send using `MPI_Isend`, utilising a data structure composed of (coordinates.x, coordinates.y, value) to adjacent processes defined by the _Moore Neighbourhood_ #footnote[In cellular automata, the Moore neighbourhood is defined on a two-dimensional square lattice and is composed of a central cell and the eight cells that surround it.] herein referenced as a coordinate-value pair.
  - Then, all processes receive the information they were sent.
  - After that, all processes modify the ghost cells at their border as required.
- A major refactor was conducted to extract common functionality.

== Enhancements

#let total_seq = 10043149294
#let total_par_old = 16104186055
#let com_par_old = 42451052

From testing these aforementioned changes, it was apparent that the code worked, but the performance was suboptimal and significantly #footnote[On the order of #calc.round(total_seq / total_par_old * 100, digits: 2)%.] slower than the sequential version.
This was mainly due to a very large proportion of time being taken up by communication and idle time#footnote[The computation time to total time ratio was of  about #calc.round(com_par_old / total_par_old * 100, digits: 2)%.].
  
Taking that into account, the code was changed so that now the processes first build a list of messages to send each other node and then send an array of coordinate-value pairs. This substantially reduces communication time by reducing the MPI communication overhead.

However, the main optimisation was implementing a cache for _global tag-to-local_ coordinate translation. This cache was used when receiving incoming coordinate–value pairs from Moore-adjacent processes. The former implementation was a brute-force approach that tried all possible local coordinates in order to obtain the corresponding global tag and return the appropriate mentioned coordinates.

#figure(
  grid(
    columns: (auto, auto),
    column-gutter: 2em,
    row-gutter: 0.25em,
    image("images/global_tag.png"),
    image("images/local_coordinates.png"),
    [(a)],[(b)]
  ),
  caption: [
    (a) Shows the "Global Tag" distribution for $"'process_count'"=9$ and $"'inner_width'"="'inner_heigth'"=1$.
    (b) Shows the "Local Coordinates" for $"'process_count'"=9$ and $"'inner_width'"="'inner_heigth'"=1$.
  ]
)
  
Another issue was that the program occasionally hung after execution. This was caused by varying process speeds; during time-bounded execution, some processes would perform an extra iteration and then hit an `MPI_Barrier` inside the main loop. Since other processes had already exited the loop, the program would never finish. The solution was to add more communication: the master process#footnote[The one at rank 0 on `MPI_COMM_WORLD`.] now communicates to the other nodes after each iteration whether another iteration should be performed.

#pagebreak()

= Experiment Design <exp-design>

/*
Entrega 3:
- Tamaños considerados para el conjunto de datos.
- Cantidades de nodos computacionales sobre los que variarán los experimentos.
- Métricas a contemplar (indicando para cada una la forma en la que planean medirla y evaluarla).
- Criterios bajo los cuales establecieron este diseño experimental.
- Objetivos de los experimentos, o expectativas que tienen en cuanto a los resultados que obtendrán.

- poner tamaños,
- time steps 
*/

== Parameters

To evaluate the performance and scalability of this parallel implementation of Conway's Game of Life, a series of experiments varying several parameters were conducted. The parameters are described in following sections.

=== Sequential Implementation Parameters

@seq-param describes the parameters applicable to the sequential algorithm.

#figure(
  table(columns: 2, align: (center, left),
    [*Property*   ], [*Description*],
    [Grid Size (X)], [Width of the grid.],
    [Grid Size (Y)], [Height of the grid.],
    [Iterations   ], [Number of generations to simulate.]
  ),
  caption: [Parameters to the sequential algorithm.]
) <seq-param>

=== Parallel Implementation Parameters

@par-param describes the parameters applicable to the parallel algorithm.

#figure(
  table(columns: 2, align: (center, left),
    [*Property*      ], [*Description*],
    [Grid Size (X)   ], [Width of the grid.],
    [Grid Size (Y)   ], [Height of the grid.],
    [Process Count   ], [Number of processes used for parallel execution.],
    [Inner Grid Width], [Width of the inner grid handled by each process.],
    [Inner Grid Height], [Height of the inner grid handled by each process.],
    [Iterations      ], [Number of generations to simulate.],
    [Runtime         ], [The total runtime of the simulation.],
  ),
  caption: [Parameters for the parallel algorithm.]
) <par-param>

Of note it is to state that the grid size is fully determined by the process count and the inner grid sizes via @size-eq for the case of a square.

#math.equation(
  [$S_"grid" = S_"inner" dot sqrt(n)$],
  numbering: "(1)",
  block: true
) <size-eq>

#pagebreak()

=== Run Configurations <sweeps>

Two distinct sweeps were conducted to evaluate the performance of the parallel implementation of the Game of Life under various configurations.

The first sweep focused on comparing different RLE (Run Length Encoded) #footnote[RLE files are explained in @appendix-1.] patterns and the impact of varying process counts on a fixed grid size. @sweep1 summarises the properties and values used in Sweep 1.


#figure(
  table(columns: 2, align: (center, left),
    [*Property*       ], [*Values*],
    [RLE              ], [
      - "mini.rle"
      - "c2-orthogonal.rle"
      - "c4-diag-switch-engines.rle"
      - "diag-glider.rle"
      - "glider.rle"
    ],
    [Processes Count  ], [-1, 1, 4],
    [Inner Width      ], [100],
    [Inner Height     ], [100],
    [Execution Index  ], [0, 1, 2, 3, 4],
    [Time (in minutes)], [3]
  ),
  caption: [Sweep 1 characteristics.]
) <sweep1>

#figure(
  image("images/W&B_file.png"),
  caption: [
    The sweep detailed in @sweep1 as a parallel coordinates plot.
  ]
)

#pagebreak()

The second sweep aimed to examine the effects of different grid sizes and process counts on the execution performance using a single RLE pattern. @sweep2 below outlines the properties and values used in Sweep 2.

#figure(
  table(columns: 2, align: (center, left),
    [*Property*       ], [*Values*],
    [RLE              ], ["mini.rle"],
    [Processes Count  ], [-1, 1, 4, 9, 16],
    [Inner Width      ], [10, 100, 500, 1000],
    [Inner Height     ], [10, 100, 500, 1000],
    [Execution Index  ], [0, 1, 2, 3, 4],
    [Time (in minutes)], [3]
  ),
  caption: [Sweep 2 characteristics.]
) <sweep2>


#figure(
  image("images/W&B_scaling.png"),
  caption: [
    The detailed in @sweep2 as a parallel coordinates plot.
  ]
)

In the previous tables, the '-1' value for _Processes Count_ indicates that the sequential version of the algorithm was run. This serves as a baseline to compare the performance of the parallel implementation against the traditional single-process execution. Each test was run five times to ensure the results' reliability and consistency. The Execution Index parameter indicates which run of the test it is, ranging from 0 to 4.

The decisions behind these choices are further explained in @objectives.

== Metrics

This section details the metrics used to evaluate the performance and scalability of this parallel implementation.

- *Steps executed*: The total number of computational steps completed.
- *Steps per core*: The average number of steps executed by each processing core.

To assess how well the workload is distributed across the computing resources, the following load balancing metrics are used:

- *Total Execution Time $T$*: The total time taken to complete the computation.
- *Time per cell per step $T_"cs"$*: The average time spent on each cell for each step. This can be calculated using @tpcs where $"Size".x$ is the total simulation width, $"Size".y$ is the total simulation height and $I$ is the number of iterations – generations.
  #math.equation(block: true, numbering: "(1)")[
    $T_"cs" = T/("Size".x dot "Size".y dot I)$
  ] <tpcs>
#block(breakable: false)[
  -  *Time per cell per step per core $T_"csc"$*: The average time spent on each cell for each step, divided by the number of cores. Computation of this metric can be done using @tpcsc, where $n$ is the number of processes.
  #math.equation(block: true, numbering: "(1)")[
    $T_"csc" = T/(n dot "Size".x dot "Size".y dot I)$
  ] <tpcsc>
]
#block(breakable: false)[
  - *Communication Time $C$*: The time spent on inter-process communication. This can be computed as the time spent on step 8 of @gol-par. @tc describes how this can be computed and summed for each process as the time from when communication starts up to when communication ends.
  #math.equation(block: true, numbering: "(1)")[
    $C = sum _"Processes" C_"end" - C_"start"$
  ] <tc>
]
#block(breakable: false)[
  - *Idle Time $O$*: The total time that processing cores spend idle. @to describes idle time where $W_"other"$ represents the time from when the current process finishes communication up to when all processes finish communication.
  #math.equation(block: true, numbering: "(1)")[
    $O = C + W_"other"$
  ] <to>
]

The scalability of the parallel implementation, is measured by:

#block(breakable: false)[
- *Speedup $S$*: The ratio of the execution time of the sequential algorithm to the parallel algorithm. It is calculated with @speedup, where $"Ips"_s$ and $"Ips"_p$ are the number of iterations-per-second of the sequential and parallel model, respectively. 
  #math.equation(block: true, numbering: "(1)")[
    $S = ("Ips"_s)/("Ips"_p)$
  ]<speedup>
]
- *Efficiency*: The speedup divided by the number of processes.

== Objectives <objectives>

This subsection explains the reasoning behind choosing the parameters and metrics described in the previous sub-sections.

To determine if the computation time is independent of the initial live cell count, experiments were run with different Run Length Encoded (RLE) patterns: one with a basic input having live cells in the single digits ("mini.rle") and another in several hundreds ("c4-diag-switch-engines.rle").
Furthermore, the process count was varied while keeping the grid size constant to gauge the strong scaling of the implementation.
Conversely, for weak scaling, both the number of processes and the grid size were increased proportionally.
Moreover, rectangular inner grid sizes were tested to observe their effects on performance. Finally, communication time was measured to understand its impact on overall performance, and idle time was monitored with the aim of quantifying the periods when processes were not actively computing.

// -----------------------------------------------------------------------------------

/*
La entrega a realizar consiste en un pequeño informe que debe incluir:

- El análisis de rendimiento realizado en base a los experimentos propuestos en el diseño experimental.
- Deben presentarse los datos generados a partir de las ejecuciones, organizados en tablas, y graficados.
- Para cada índice de rendimiento debe incluirse el análisis, la reflexión realizada y las conclusiones a las que han podido llegar. 
- La documentación de posibles modificaciones en el código y/o en los experimentos, que consideren les permitirá mejorar el rendimiento del programa, y/o analizar el comportamiento desde otro punto de vista (por ejemplo: aislando secciones limitantes, cambiando estrategias, mejorando alguna política, etc.). Pueden proponer todas las mejoras que consideren, aunque se trabaje sólo en alguna de ellas, dadas las restricciones de tiempo del semestre. 
*/

= Performance Analysis <performance>

== Execution Environment

The sweeps described previously described in @sweeps were run on only one laptop  ("DGPC"), its characteristics are detailed in @DGPC. 

#figure(
  table(columns: 2, align: (center, left),
    [*Property*         ], [*Values*],
    [Architecture       ], [x86_64],
    [CPU Model Name     ], [11th Gen Intel Core i7-11800H \@ 2.30GHz],
    [Thread(s) per core ], [2],
    [Core(s) per socket ], [8],
    [CPU(s)             ], [16],
    [L1d cache size     ], [384 KiB (8 instances)],
    [L1i cache size     ], [256 KiB (8 instances)],
    [L2 cache size      ], [10 MiB (8 instances)],
    [L3 cache size      ], [24 MiB (1 instance)]
  ),
  caption: [DGPC characteristics.]
) <DGPC>

== Analysis

/*
- Deben presentarse los datos generados a partir de las ejecuciones, organizados en tablas, y graficados.
- Para cada índice de rendimiento debe incluirse el análisis, la reflexión realizada y las conclusiones a las que han podido llegar.
*/

The following analysis was done with results obtained from the *second sweep* execution (@sweep2).

As described previously, to evaluate the _weak scaling_, the number of processes and the grid size were increased proportionally. In the resulting graphs, it can be appreciated how the speed up improves with the number of cells per process. On the other hand, efficiency decreases with an increasing number of processes. However, the rate of this decrease diminishes as the number of cells per process rises.

#figure(
  image("images/results/weak_scale_2.png"),
  caption: [Speedup obtained when evaluating weak scaling.]  
) <weak-speed>

#figure(
  image("images/results/weak_scale_1.png"),
  caption: [Efficiency obtained when evaluating weak scaling.]  
) <weak-efficiency>

// Conversely, to evaluate the _strong scaling_ process count was varied while keeping the total grid size constant.

// #to-do[speedup]
// #to-do[efficiency]

@ips-os compares the outer (total) size with the achieved number of millisecond per iterations. The number of milliseconds per iterations increases with the outer size, and it can be seen that increasing number of nodes increases performance.

#figure(
  image("images/results/linear_in_os.png"),
  caption: [Scatter plot comparing the milliseconds per iterations with the outer size.]  
) <ips-os>

Using the results from the *first sweep* (@sweep1), the deviation in iterations-per-second from those obtained with the initial state representation "mini.rle" was calculated. The deviation was found to be less than 1%, leading to the conclusion that computation time is independent of the initial live cell count provided by different Run Length Encoded (RLE) patterns.

#figure(
  image("images/results/file.png"),
  caption: [Bar chart comparing the iterations-per-second relative to the "mini.rle".]  
)

@comm-idle illustrates the percentage of time allocated to computation, communication, and idle states across the *two sweeps*. It is evident that as the number of processes increases, the time spent on communication and idle states also increases. However, this time does not exceed the time spent on computation.

#figure(
  image("images/results/communication.png"),
  caption: [Bar chart comparing allocated to computation, communication, and idle states.]  
) <comm-idle>


/*
- La documentación de posibles modificaciones en el código y/o en los experimentos, que consideren les permitirá mejorar el rendimiento del programa, y/o analizar el comportamiento desde otro punto de vista (por ejemplo: aislando secciones limitantes, cambiando estrategias, mejorando alguna política, etc.). Pueden proponer todas las mejoras que consideren, aunque se trabaje sólo en alguna de ellas, dadas las restricciones de tiempo del semestre. 
*/

// - We could try to avoid computing empty areas
//   - If we do, We could add load balancing
// - We could not compute the state of ghost atoms
// - We could have multiple layers of ghost atoms


= Conclusions <conclusion>

/*
En las conclusiones cada grupo debe enumerar las conclusiones a las que ha arribado, a la luz de los resultados obtenidos y del análisis efectuado sobre los mismos.

En esta sección pueden mencionarse y listarse posibles mejoras para solventar los problemas o debilidades que hayan podido detectar durante el análisis. Deben estar debidamente justificadas.
*/

Overall, the results obtained showcase an improvement over the sequential implementation. However, further experiments are needed to provide a more comprehensive comparison. 

In future implementations, expanding the scope to include experiments across different hardware configurations could offer valuable insights into the system's performance variability.

== Possible Improvements

To enhance the performance and efficiency of this parallel implementation, several potential improvements can be considered:

- Implementing a strategy to *skip computations in areas without live cells* could significantly reduce unnecessary processing. If this approach is adopted, integrating a load-balancing mechanism would ensure an even distribution of computational workload across processes.

- *Not computing the state of ghost atoms*, streamlining the algorithm and reducing computational overhead.

- *Adding multiple layers of ghost atoms* might improve the efficiency of boundary cell computations, potentially leading to better overall performance as more iterations may be accomplished without the need to communicate.


= References

#bibliography("refs.bib")

// #to-do[fix references. (los links tienen q mostrar cuando accedimos)]

// -----------------------------------------------------------------------------------

#pagebreak()

#let appendix(body) = {
  set heading(numbering: "I.", supplement: [Appendix])
  counter(heading).update(0)
  body
}

#appendix[

#show raw: x=>box(
inset: (x: 0.3em),
box(fill: luma(0).transparentize(90%), outset: 0.2em, inset: (x: 0.1em), radius: 0.2em, x))
  
= Appendix <appendix>

== Initial State Representation <appendix-1>

The file format utilised to store the initial state is RLE (Run Length Encoding).  RLE is a compact and efficient method used to represent patterns or configurations within Game of Life. An example of this format is shown in @sample-rle.

#figure(caption: "Sample RLE file.", supplement: "File")[
  #sourcecode[```r
    # Sample RLE file.
    x = 9, y = 5, rule = B3/S23
    $bo3b3o$b3o2bo$2bo!
  ```
  ]
] <sample-rle>

#figure(
  image("images/RLE-Picture.png", width: 100%),
  caption: [The plotted pattern of @sample-rle.])

In this format, each line is delimited by '`$`'. Within each line, the cells are represented as a sequence of characters where '`b`' denotes a black o dead cell, and '`o`' denotes a white or alive cell. Additionally, numeric characters are used to indicate the number of consecutive cells with the same state. For example, '`3o`' indicates three live cells in a row, and '`2b`' represents two dead cells in a row.

RLE encoding also includes metadata such as the width and height of the pattern, as well as any additional rules that may apply to the pattern's evolution. This metadata ensures that the pattern can be correctly interpreted and simulated within the rules.

#pagebreak()

== Build and Execution <appendix-2>

This project *requires* C++ standard version 23, and at least CMake 3.28. Additionally, it is necessary an MPI-4.1 compatible implementation of MPI.

#heading(outlined: false, level: 3, [Build])

The steps to compile the previously downloaded source code are outlined below:

// + Download the source code.
+ Navigate to the source root.
+ Create a new directory named `build`.
+ `cd` into that directory.
+ Run `cmake ..`.
+ Run `make -j $(nproc)`.

This last command compiles the project using `mpic++` and `std=c++23` for the available number of processors. The resulting compiled binary should be located in `build/src/main/gol`. 

The sequential version can be executed with: `build/src/main/gol`.

Meanwhile, the parallel version can be run with: 
#align(center)[`mpirun -n <number of processes> build/src/main/par_gol`]

//#heading(outlined: false, level: 3, [Sequential Execution])

The *sequential execution arguments* are as follows:

#align(center)[`src/main.o <filename> <steps_to_simulate> <delay_microseconds>`]

Where the default values are:

- Step count: 0.
- Delay: $50000 mu s = 50 m s$.

The final program can be customised modifying the following predefined constants (defines) in the source code:

- `S_WIDTH`: Simulation width.
- `S_HEIGHT`: Simulation height.
- `PRINT`: Verbose logging.

== Experiments Execution

This section provides instructions on how to set up and run the experiments using _Weights and Biases (WANDB)_ for tracking and managing the progress. Thus, having a WANDB account is required, such can be created at "wandb.ai".

Having previously created a WANDB account, the steps are as follows:

+ Install WANDB locally:

  Use pip to install WANDB by running the following command:

  ```sh
  pip install wandb
  ```
#pagebreak()

+ Login to WANDB:

  Run the following command to log in to WANDB and paste in your token when prompted:

  ```sh
  wandb login
  ```
  
+ Create a `sweep.yaml`:

  Define your sweep configuration in a file named `sweep.yaml`. This file will contain the parameters and configurations for your experiments.

+ Create the sweep:

  Use the following command to create the sweep in your WANDB project. Replace `<project>` with your project name and `<entity>` with your username or team name:

  ```sh
  wandb sweep -p <project> -e <entity> sweep.yaml
  ```
  
+ Prepare the executor environment:

  Create a file named `available_processes.txt` inside the executor directory. This file should list the number of cores available on the machine.

+ Run the sweep agents:

  From the executor directory, run the sweep agent command as many times as needed to match the available cores. Use the `nohup` command to ensure the agents continue running in the background:

  ```sh
  nohup <sweep agent cmd> &
  ```
]

/*
si mejoramos comunicaciones lo mencionamos después.

deicr como se mide el speedup: iteraciones por segundo.

presentación:
- el problema
  - el golly
- mostramos el pseudocódigo
- resumen ppales decisiones (desco por dom, master-worker, com, matrices, vecinos, cosas que impactaron en las decisiones)
- métricas
- ppales desafios de la implementación
- resultados e interpretación
- aclarar como medimos el t(1)
*/

/*
entre martes y jueves presentación
15 min - no más de 20
podemos usar el informe como presentación (no me pinta) ok cada vez me pinta más
la presentación podemos entregar después
*/


