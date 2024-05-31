#include <mpi.h>
#include <chrono>
#include "../include/canvas.h"
#include "../include/mpi_utils.h"
#include "../include/mpi_canvas.h"
#include "../include/arg_parse.h"
#include "../include/mpi_arg_parse.h"
#include "../include/mpi_canvas_debug.h"

using namespace std;
using namespace chrono;

bool print = true;


OutputValues time_based(MPICanvas &canvas, long millis) {
  auto start = high_resolution_clock::now();
  unsigned long long its = 0;
  unsigned long long comm_time = 0;
  unsigned long long compute_time = 0;
  unsigned long long idle_time = 0;

  while (true) {
    auto comm_start = high_resolution_clock::now();
    canvas.comunicate();
    auto comm_end = high_resolution_clock::now();
    MPI_Barrier(MPI_COMM_WORLD);
    auto compute_start = high_resolution_clock::now();
    canvas.canvas->iter();
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    its++;
    comm_time += duration_cast<nanoseconds>(comm_end - comm_start).count();
    idle_time += duration_cast<nanoseconds>(compute_start - comm_end).count();
    compute_time += duration_cast<nanoseconds>(end - compute_start).count();
    bool will_break = duration.count() >= millis;
    MPI_Bcast(&will_break, 1, MPI_CXX_BOOL, 0, MPI_COMM_WORLD);
    if (will_break) break;
  }
  auto end = high_resolution_clock::now();
  auto duration = duration_cast<nanoseconds>(end - start);
  return OutputValues{
   canvas.rank, its, (unsigned long long) duration.count(),
   comm_time, idle_time, compute_time
  };
}


OutputValues iter_based(MPICanvas &canvas, unsigned long long iter_count) {
  auto start = high_resolution_clock::now();
  unsigned long long comm_time = 0;
  unsigned long long compute_time = 0;
  unsigned long long idle_time = 0;
  for (unsigned long long i = 0; i < iter_count; i++) {
    auto comm_start = high_resolution_clock::now();
    canvas.comunicate();
    auto comm_end = high_resolution_clock::now();
    MPI_Barrier(MPI_COMM_WORLD);
    auto compute_start = high_resolution_clock::now();
    canvas.canvas->iter();
    auto end = high_resolution_clock::now();
    comm_time += duration_cast<nanoseconds>(comm_end - comm_start).count();
    idle_time += duration_cast<nanoseconds>(compute_start - comm_end).count();
    compute_time += duration_cast<nanoseconds>(end - compute_start).count();
  }
  auto end = high_resolution_clock::now();
  auto duration = duration_cast<nanoseconds>(end - start);

  return OutputValues{
   canvas.rank, iter_count, (unsigned long long) duration.count(),
   comm_time, idle_time, compute_time
  };
}

void program_main(int argc, char **argv) {
  Args args = MPIArgParse::parse(argc, argv, print);
  MPIUtils::init_datatypes();
  MPICanvas canvas = MPICanvas(
   new Canvas(args.width + 2, args.height + 2),
   Vec2<unsigned int>{args.height, args.height},
   Vec2<unsigned int>{1, 1}
  );
  args.height = canvas.item_size.y * canvas.dims.y;
  args.width = canvas.item_size.x * canvas.dims.x;
  canvas.canvas->init();
  if (print) cout << "state: Compiling..." << endl;
  Vec2<unsigned int>::pre_build(canvas.cart_coords, canvas.item_size, canvas.dims);
  if (print) cout << "state: Loading..." << endl;
  canvas.load_file(args.file, 1, 1);
  MPI_Barrier(MPI_COMM_WORLD);
  if (print) cout << args << flush;
  OutputValues ov = args.type ? time_based(canvas, args.millis) : iter_based(canvas, args.iter_count);
  cout << MPIOutputValues::output(ov);
  MPI_Type_free(&MPIUtils::MPI_Vec2);
  MPI_Type_free(&MPIUtils::MPI_CoordsValue);
}

int main(int argc, char **argv) {
  try {
    MPI_Init(&argc, &argv);
    print = MPIUtils::get_rank() == 0;
    program_main(argc, argv);
    MPI_Finalize();
    return 0;
  } catch (const runtime_error &e) {
    if (print) cerr << e.what() << endl;
    MPI_Finalize();
    return 1;
  }
}

