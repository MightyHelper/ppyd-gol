#include <mpi.h>
#include "../include/canvas.h"
#include "../include/mpi_utils.h"
#include "../include/mpi_canvas.h"
#include "../include/mpi_canvas_debug.h"

using namespace std;

int main() {
  MPI_Init(nullptr, nullptr);
  MPIUtils::init_datatypes();
  MPICanvas canvas = MPICanvas(
    new Canvas(82, 82),
    Vec2<unsigned int>{80, 80},
    Vec2<unsigned int>{1, 1}
  );
  canvas.canvas->init();
//    if (canvas.rank == 4) {
//        canvas.canvas->load_file("../data/diag-glider.rle", 1, 1);
//    }
//    if (canvas.rank == 0) {
//        canvas.canvas->load_file("../data/glider.rle", 1, 1);
//    }
  canvas.load_file("../data/c4-diag-switch-engines.rle", 40, 40);
//  Debug::print_all5(canvas, {40, 0});
  for (int i = 0; i < 1000; i++) {
    canvas.comunicate();
    MPI_Barrier(MPI_COMM_WORLD);
    Debug::print_all5(canvas, {40, 0});
    canvas.canvas->iter();
//    usleep(100000);
  }
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Type_free(&MPIUtils::MPI_Vec2);
  MPI_Type_free(&MPIUtils::MPI_CoordsValue);
  MPI_Finalize();
  return 0;
}

