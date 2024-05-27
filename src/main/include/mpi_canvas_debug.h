#ifndef GOL_MPI_CANVAS_DEBUG_H
#define GOL_MPI_CANVAS_DEBUG_H


#include "mpi_canvas.h"
#include <sstream>
#include <functional>

namespace Debug {

  void print_all(const MPICanvas &canvas);

  void print_all_generic(
   const MPICanvas &canvas,
   Vec2<unsigned int> base,
   Vec2<unsigned int> delta,
   const std::function<void(std::stringstream &, Vec2<unsigned int>, Vec2<unsigned int>)> &f
  );

  void print_all_full_generic(
   const MPICanvas &canvas,
   Vec2<unsigned int> base,
   Vec2<unsigned int> delta,
   const std::function<void(std::stringstream &, Vec2<unsigned int>, Vec2<unsigned int>)> &f
  );
  void print_all_full_generic_line(
   const MPICanvas &canvas,
   Vec2<unsigned int> base,
   Vec2<unsigned int> delta,
   const std::function<void(std::stringstream &, Vec2<unsigned int>, Vec2<unsigned int>)> &f
  );

  void print_all5(const MPICanvas &canvas, Vec2<unsigned int> base = {0, 0});

  void print_all5_full(const MPICanvas &canvas, Vec2<unsigned int> base = {0, 0});

  void print_all_full_all(const MPICanvas &canvas, Vec2<unsigned int> base = {0, 0});

  void debug_ranks(const MPICanvas &canvas);

  void debug_neighbors(const MPICanvas &canvas);

  void plot_topo(const MPICanvas &canvas);

  void debug_mpi_datatypes(const MPICanvas &canvas);

}


#endif //GOL_MPI_CANVAS_DEBUG_H
