#include <iostream>
#include <chrono>
#include "../include/arg_parse.h"

using namespace std;



void Args::print_usage(const char *name, bool print) {
  if (print) {
    cout << "Usage: " << name << " <file>" << endl;
    cout << "       " << name << " <file> f iterations   width height [delay=0]" << endl;
    cout << "       " << name << " <file> t milliseconds width height [delay=0]" << endl;
  }
  throw runtime_error("Invalid arguments.");
}

Args Args::parse(int argc, char **argv, bool print) {
  if (argc == 0 or argc == 1) throw runtime_error("No arguments provided.");

  Args args;
  args.file = argv[1];
  if (argc == 2) return args;
  if (argc == 3) Args::print_usage(argv[0], print);
  args.type = argv[2][0] == 't';
  if (args.type) args.millis = stol(argv[3]);
  else args.iter_count = stol(argv[3]);
  if (argc >= 5) args.width = stoull(argv[4]);
  else args.width = 100;
  if (argc >= 6) args.height = stoull(argv[5]);
  else args.height = 100;
  if (argc > 6) Args::print_usage(argv[0], print);
  args.inner_width = args.width;
  args.inner_height = args.height;
  args.program_type = "sequential";
  args.num_mpi_threads = 0;
  args.num_threads = 1;
  return args;
}
