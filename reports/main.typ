// Consigna:
// 1. Breve descripción del problema a resolver.
// 2. Pseudocódigo propuesto (correspondiente al programa secuencial desarrollado)
// 3. Análisis de las posibilidades de paralelización que identifican en la solución (propuesta de paralelización).
// 4. Código fuente de la versión secuencial.
//  => Le pregunte a la profe y no hace falta copiar y pegar el codigo aca, podemos poner un par de lineas estilo algoritmo, el codigo real va adjunto.
// 5. [Done] Por favor indicar el nombre y apellido de todos los integrantes del grupo.

// Code-style guidelines
// - I suggest you use VS Code + Typst plugin (Or the typst web app)
// - New sentences in a paragraph should start on a new line (You might have to indent it with spaces to get it to not wrap)
// - I wrote this template myself so please be kind :3
// - TBD

// Interesting links
// https://conwaylife.com/wiki/

#import "@preview/algorithmic:0.1.0"
#import algorithmic: algorithm



#let author(name, organization, organization_group) = [
  #text(weight: "bold", name)

  #organization

  #organization_group
]
#let to-verify(content) = [
  #text(fill:orange, content) // Wrap stuff that will need revision later on with this.
  #text(fill:red, weight: "bold")[\- To be reviewed]
]
#set text(font: "Times New Roman") // Set default font

#show heading: x=> smallcaps[#x] // Set all headings to use SmallCaps :3
#set heading(numbering: "I.I.")
#set par(
  first-line-indent: 1em,
  justify: true,
  linebreaks: "optimized"
)
#set page(
  paper: "a4",
  numbering: "1" // We could use "1/1" to add the total page count :3
)

#align(center)[
  #heading(outlined: false, numbering: none, level:1, [MPI implementation of Conway's Game of Life])
  #v(1em)
  #grid(columns: (1fr, 1fr),
    author(
      [Micaela Del Longo],
      [Faculty of Engineering],
      [National University of Cuyo (UNCUYO)]
    ),
    author(
      [Federico Williamson],
      [Faculty of Engineering],
      [National University of Cuyo (UNCUYO)]
    ),
  )
]
#columns(2, [
#heading(level: 1, numbering: none, outlined: false)[#text(style: "italic")[Abstract]]
#text(weight: "bold")[
  Conway's Game of Life@gardner1970mathematical, a classic cellular automaton, presents intriguing challenges for parallelization due to its inherently sequential nature. In this work, we propose a parallel implementation of Conway's Game of Life using the Message Passing Interface (MPI) paradigm. Our approach aims to exploit the inherent parallelism of the game to efficiently distribute computation across multiple processes, thereby harnessing the power of modern parallel computing architectures.
]

= Introduction

We begin by describing the sequential version of the Game of Life and identifying potential bottlenecks in its parallelization. We then introduce our MPI-based parallelization strategy, which involves decomposing the game grid into smaller subgrids and distributing them among MPI processes. We employ efficient communication schemes to enable inter-process coordination and synchronization while minimizing overhead.

#to-verify[
  To evaluate the performance of our MPI implementation, we conduct a series of experiments on various parallel computing platforms, ranging from multi-core CPUs to distributed memory clusters. We analyze scalability, load balancing, and overhead characteristics under different problem sizes and MPI configurations. Our results demonstrate significant speedup compared to the sequential version, highlighting the effectiveness of MPI in harnessing parallelism in Conway's Game of Life.
]
= Game of Life
Conway's Game of Life, devised by mathematician John Conway in 1970, is a cellular automaton that operates on a grid of cells, each of which can be in one of two states: _alive or dead_.
The game evolves according to a set of simple rules based on the concept of generations.

- *Birth*: A dead cell with exactly three live neighbors#footnote[A neighbor is defined as any adjacent cell, including diagonals.] becomes alive (is "born") in the next generation.

- *Survival*: A live cell with two or three live neighbors remains alive in the next generation.
  This reflects the idea of "survival of the fittest" among neighboring cells.

- *Death by Isolation*: A live cell with fewer than two live neighbors dies due to underpopulation in the next generation, as if by loneliness.

- *Death by Overcrowding*: A live cell with more than three live neighbors dies due to overcrowding in the next generation, as if by lack of resources.

These rules are applied simultaneously to every cell in the grid for each generation.
The game progresses in discrete time steps, with each step generating a new configuration of live and dead cells based on the current state of the grid.
The resulting patterns can exhibit a wide range of behaviors, including static patterns, oscillations, and complex evolving structures.
Despite its simple rules, the Game of Life can produce remarkably intricate and unpredictable dynamics, making it a fascinating subject of study in mathematics, computer science, and artificial life.

= Sequential Pseudocode
To provide a clear understanding of the sequential implementation of Conway's Game of Life, we present pseudocode detailing the algorithm's key steps.
The following pseudocode outlines the sequential evolution of the game grid from one generation to the next:
])

#figure(
  supplement: "Algorithm",
  placement: top,
  kind: "code",
  caption: "Game of life Algorithm",
  align(left,
    block(breakable: false, fill: luma(230), inset: 1em, radius: 0.5em,
      algorithm({
        import algorithmic: *
        let myCmt = x=>Cmt(text(fill: rgb(56, 76, 107), x))
        Function("Conway-Game-of-Life", args: ("grid", ), {
          myCmt[Create a new grid to hold the next generation]
          Assign[$"nextGrid"$][#FnI[$"createGridOfSize"$][$"grid.size"$]]

          myCmt[Iterate over each cell in the grid]
          For(cond: $"cell in grid"$, {
            myCmt[Count the number of live neighbors for the current cell]
            Assign[$"liveNeighbors"$][FnI[$"countLiveNeighbors"$][grid, cell]]

            myCmt[Apply the rules of the Game of Life]
            If(cond: $"cell.isAlive"$, {
              If(cond: $"liveNeighbors" < 2 || "liveNeighbors" > 3$, {
                myCmt[Cell dies due to underpopulation or overcrowding]
                Assign[$"nextGrid"["cell.position"]$][*DEAD*]
              })
              Else({
                myCmt[Cell survives to the next generation]
                Assign[$"nextGrid"["cell.position"]$][*ALIVE*]
              })
            })
            Else({
              If(cond: $"liveNeighbors" == 3$, {
                myCmt[Dead cell becomes alive due to reproduction]
                Assign[$"nextGrid"["cell.position"]$][*ALIVE*]
              })
              Else({
                Assign[$"nextGrid"["cell.position"]$][*DEAD*]
              })
            })
          })
          // Update the original grid with the next generation
          Assign($"grid"$, $"nextGrid"$)

          // Return the updated grid
          Return($"grid"$)
        })
      })
    )
  )
) <gol-algoritm>

#columns(2, [
In @gol-algoritm, the evolve function takes the current grid state as input and returns the grid state after one generation. It iterates over each cell in the grid, counts the number of live neighbors for each cell using the countLiveNeighbors function, and applies the rules of the Game of Life to determine the state of each cell in the next generation. The resulting grid represents the next generation of the game.

This sequential algorithm forms the basis for parallelization using Message Passing Interface (MPI), where the grid is divided among multiple processes to enable concurrent computation and evolution of the game.
])

#pagebreak(weak: true)
#bibliography("refs.bib")