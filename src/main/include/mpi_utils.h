#ifndef GOL_MPI_UTILS_H
#define GOL_MPI_UTILS_H

#include <mpi.h>
#include <string>
#include <vector>
#include "vec2.h"

#define validateMPIOutput(value) MPIUtils::_validateMPIOutput(value)

namespace MPIUtils {

    extern MPI_Datatype MPI_CoordsValue;
    extern MPI_Datatype MPI_Vec2;

    int get_rank();
    int get_size();


    void MPI_Gather_string(const std::string &send, std::string &recv, int root, MPI_Comm comm);

    void _validateMPIOutput(int output);

    void init_datatypes();

    void recv_request(std::vector<MPI_Request> &recv_requests, std::vector<CoordsValue *> &recv_data);
    void send_request(std::vector<MPI_Request> &send_requests, std::vector<CoordsValue *> &send_data, unsigned int dest, CoordsValue *cv);

};


#endif //GOL_MPI_UTILS_H
