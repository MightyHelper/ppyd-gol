#ifndef GOL_ARG_PARSE_H
#define GOL_ARG_PARSE_H

#include <iostream>
#include <climits>
#include <unistd.h>

static const int NS_TO_MS = 1000000;

struct Args {
  std::basic_string<char> file;
  bool type = false; // 0 = iterations; 1 = time
  long iter_count = 0;
  long millis = 0;
  unsigned int width = 100;
  unsigned int height = 100;
  unsigned int inner_width = 100;
  unsigned int inner_height = 100;
  std::string program_type;
  unsigned int num_mpi_threads = 0;
  unsigned int num_threads = 0;
  static Args parse(int argc, char **argv, bool print);
  static void print_usage(const char *name, bool print);

  friend std::ostream &operator<<(std::ostream &os, const Args &args) {
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    os << "run_config:" << '\n';
    os << "  file: " << args.file << '\n';
    os << "  objective:" << '\n';
    os << "    type: " << args.type << '\n';
    os << "    iter_count: " << args.iter_count << '\n';
    os << "    millis: " << args.millis << '\n';
    os << "  size:" << '\n';
    os << "    width: " << args.width << '\n';
    os << "    height: " << args.height << '\n';
    os << "    inner_width: " << args.inner_width << '\n';
    os << "    inner_height: " << args.inner_height << '\n';
    os << "  environment:" << '\n';
    os << "    hostname: " << hostname << '\n';
    os << "    program_type: " << args.program_type << '\n';
    os << "    num_mpi_threads: " << args.num_mpi_threads << '\n';
    os << "    num_threads: " << args.num_threads << '\n';
    return os;
  }
};
struct OutputValues {
  unsigned int process_rank;
  unsigned long long iterations;
  unsigned long long total_time_ns;
  unsigned long long communication_time_ns;
  unsigned long long idle_time_ns;
  unsigned long long compute_time_ns;

  friend std::ostream &operator<<(std::ostream &os, const OutputValues &values) {
    os << "- rank: " << values.process_rank << '\n';
    os << "  results:" << '\n';
    os << "    iterations: " << values.iterations << '\n';
    os << "    time:" << '\n';
    os << "      total: " << ((double) values.total_time_ns) / NS_TO_MS << '\n';
    os << "      communication: " << ((double) values.communication_time_ns) / NS_TO_MS << '\n';
    os << "      idle: " << ((double) values.idle_time_ns) / NS_TO_MS << '\n';
    os << "      compute: " << ((double) values.compute_time_ns) / NS_TO_MS << '\n';
    return os;
  }
};
#endif //GOL_ARG_PARSE_H
