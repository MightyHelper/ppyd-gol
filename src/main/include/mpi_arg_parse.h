//
// Created by federico on 5/29/24.
//

#ifndef GOL_MPI_ARG_PARSE_H
#define GOL_MPI_ARG_PARSE_H


#include "arg_parse.h"

namespace MPIArgParse {
  Args parse(int argc, char **argv, bool print);
};


namespace MPIOutputValues{
  std::string output(OutputValues ov);
}

#endif //GOL_MPI_ARG_PARSE_H
