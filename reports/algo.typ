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
              Assign[$"liveNeighbours"$][FnI[$"countLiveNeighbours"$][grid, cell]]
  
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