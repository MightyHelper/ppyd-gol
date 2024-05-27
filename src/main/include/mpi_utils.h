#ifndef GOL_MPI_UTILS_H
#define GOL_MPI_UTILS_H

#include <mpi.h>
#include <string>
#define validateMPIOutput(value) MPIUtils::_validateMPIOutput(value)

namespace MPIUtils {

    extern MPI_Datatype MPI_CoordsValue;
    extern MPI_Datatype MPI_Vec2;

    void MPI_Gather_string(const std::string &send, std::string &recv, int root, MPI_Comm comm);

    void _validateMPIOutput(int output);

    void init_datatypes();
};


#endif //GOL_MPI_UTILS_H
