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
            new Canvas(12, 12),
            Vec2<unsigned int>{10, 10},
            Vec2<unsigned int>{1, 1}
    );
    canvas.canvas->init();
    if (canvas.rank == 4) {
        canvas.canvas->load_file("../data/diag-glider.rle", 1, 1);
    }
    if (canvas.rank == 0) {
        canvas.canvas->load_file("../data/glider.rle", 1, 1);
    }
    for (int i = 0; i < 1000; i++) {
        canvas.comunicate();
		    MPI_Barrier(MPI_COMM_WORLD);
        Debug::print_all5(canvas, {40, 0});
        canvas.canvas->iter();
//        usleep(50000);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Type_free(&MPIUtils::MPI_Vec2);
    MPI_Type_free(&MPIUtils::MPI_CoordsValue);
    MPI_Finalize();
    return 0;
}

