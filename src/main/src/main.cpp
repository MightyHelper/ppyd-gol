#include "../include/canvas.h"
#include "../include/arg_parse.h"
#include <chrono>
#include <iostream>

using namespace std;
using namespace chrono;


OutputValues time_based(Canvas &canvas, long millis) {
  auto start = high_resolution_clock::now();
  unsigned long long its = 0;
  unsigned long long compute_time = 0;
  while (true) {
    auto comm_start = high_resolution_clock::now();
    canvas.iter();
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    its++;
    compute_time += duration_cast<nanoseconds>(end - comm_start).count();
    if (duration.count() >= millis) break;
  }
  auto end = high_resolution_clock::now();
  auto duration = duration_cast<nanoseconds>(end - start);
  return OutputValues{
   0, its, (unsigned long long) duration.count(),
   0, 0, compute_time
  };
}

OutputValues iter_based(Canvas &canvas, unsigned long long iter_count) {
  auto start = high_resolution_clock::now();
  unsigned long long compute_time = 0;
  for (unsigned long long i = 0; i < iter_count; i++) {
    auto compute_start = high_resolution_clock::now();
    canvas.iter();
    auto end = high_resolution_clock::now();
    compute_time += duration_cast<nanoseconds>(end - compute_start).count();
  }
  auto end = high_resolution_clock::now();
  auto duration = duration_cast<nanoseconds>(end - start);
  return OutputValues{
   0, iter_count, (unsigned long long) duration.count(),
   0, 0, compute_time
  };
}

void program_main(int argc, char **argv) {
  Args args = Args::parse(argc, argv, true);
  cout << args;
  Canvas canvas{args.width, args.height};
  canvas.load_file(args.file, 0, 0);
  OutputValues ov = args.type ? time_based(canvas, args.millis) : iter_based(canvas, args.iter_count);
  cout << "output: \n" << ov;
}

int main(int argc, char *argv[]) {
  try {
    program_main(argc, argv);
    return 0;
  } catch (const exception &e) {
    cerr << e.what() << endl;
    return 1;
  }
}
