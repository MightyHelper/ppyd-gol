//
// Created by federico on 5/29/24.
//

#include "../include/mpi_arg_parse.h"
#include "../include/mpi_utils.h"
#include <cmath>

#include <sstream>

Args MPIArgParse::parse(int argc, char **argv, bool print){
  Args args = Args::parse(argc, argv, print);
  args.width = args.inner_width;
  args.height = args.inner_height;
  args.num_mpi_threads = MPIUtils::get_size();
  args.inner_width = args.width / sqrt(args.num_mpi_threads);
  args.inner_height = args.height / sqrt(args.num_mpi_threads);
  args.program_type = "parallel";
  return args;
}

std::string MPIOutputValues::output(OutputValues ov){
  std::stringstream info;
  info << ov << '\n';
  std::string out;
  MPIUtils::MPI_Gather_string(info.str(), out, 0, MPI_COMM_WORLD);
  if (ov.process_rank == 0)
    return "output: \n" + out;
  return "";
}
