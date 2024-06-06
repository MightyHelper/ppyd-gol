#import "@preview/algorithmic:0.1.0"
#import algorithmic: algorithm

#let gol = {
  figure(
    supplement: "Algorithm",
    // placement: bottom,
    kind: "code",
    caption: "Game of Life pseudocode.",
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
              myCmt[Count the number of live neighbours for the current cell]
              Assign[$"liveNeighbours"$][#FnI[$"countLiveNeighbours"$][grid, cell]]
  
              myCmt[Apply the rules of the Game of Life]
              If(cond: $"cell.isAlive"$, {
                If(cond: $"liveNeighbours" < 2 || "liveNeighbours" > 3$, {
                  myCmt[Cell dies due to underpopulation or overcrowding]
                  Assign[$"nextGrid"["cell.position"]$][*DEAD*]
                })
                Else({
                  myCmt[Cell survives to the next generation]
                  Assign[$"nextGrid"["cell.position"]$][*ALIVE*]
                })
              })
              Else({
                If(cond: $"liveNeighbours" == 3$, {
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
  )
}


#let gol-par = {
  figure(
    supplement: "Algorithm",
    // placement: bottom,
    kind: "code",
    caption: "Parallel Game of Life pseudocode.",
    align(left,
      block(breakable: false, fill: luma(230), inset: 1em, radius: 0.5em,
        algorithm({
          import algorithmic: *
          let myCmt = x=>Cmt(text(fill: rgb(56, 76, 107), x))
          Function("Parallel-Conway-Game-of-Life", {
            myCmt[Register MPI Datatypes]
            myCmt[Initialise sub-canvas]
            myCmt[Construct 2d Cartesian Communicator]
            myCmt[Obtain adjacent processes]
            myCmt[Load initial state]
            For(cond: $"iter in steps"$, {
              myCmt[Update ghost cells]
              myCmt[Step all canvas cells]
            })
          })
        })
      )
    )
  )
}



#let _zz = ```
Hola profe, con Mica estábamos considerando con quá parámetros probar nuestros algoritmos secuenciales/parelelos y se nos ocurrió la idea de en vez de simular por X cantidad de pasos, simular por X cantidad de tiempo y guardar cuantos pasos se ejecutaron en ese tiempo definido.

De esta forma igual podemos comparar resultados entre el secuencial y el paralelo.

Estamos considerando realizar ejecuciones de 3 minutos.

Por ejemplo:
En el caso del secuencial, si simulamos un estado de 100x100, en 3 minutos podria completar (por tirar un numero) 10 000 pasos.
En el caso del paralelo, si simulamos un estado total de 100x100, con 9 procesos, en 3 minutos esperaríamos que logre alrededor de 90 000 pasos (por tirar un numero, realísticamente podría ser 50 000).

Y también podemos probar escalado fuerte, probar 300x300 con 9 procesos en 3 minutos y ver cuánto nos desviamos de 10 000 pasos.

Esto nos permitiría conocer de antemano la duracion total de nuestras simulaciones y garantizamos que no habran simulaciones muy largas ni muy cortas.
```