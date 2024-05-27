#include "../include/mpi_utils.h"
#include "../include/vec2.h"
#include <iostream>
#include <vector>

using namespace std;

void MPIUtils::MPI_Gather_string(const string &send, string &recv, int root, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    // Get the length of the string to be sent
    int send_len = (int) send.length();

    // Gather the lengths of the strings from all processes
    vector<int> recv_lens(size);
    MPI_Gather(&send_len, 1, MPI_INT, recv_lens.data(), 1, MPI_INT, root, comm);

    // Calculate displacements and total length of the concatenated string
    vector<int> displs(size);
    int total_len = 0;
    if (rank == root) {
        for (int i = 0; i < size; ++i) {
            displs[i] = total_len;
            total_len += recv_lens[i];
        }
    }

    // Gather the actual strings
    vector<char> send_buf(send.begin(), send.end());
    vector<char> recv_buf;
    if (rank == root) {
        recv_buf.resize(total_len);
    }

    MPI_Gatherv(send_buf.data(), send_len, MPI_CHAR, recv_buf.data(), recv_lens.data(), displs.data(), MPI_CHAR, root,
                comm);

    // Convert the gathered characters back into a string at the root process
    if (rank == root) {
        recv = string(recv_buf.begin(), recv_buf.end());
    }
}

void MPIUtils::_validateMPIOutput(int output) {
    if (output != MPI_SUCCESS) {
        cout << "Error: " << output << endl;
        char error_string[MPI_MAX_ERROR_STRING];
        int length_of_error_string;
        MPI_Error_string(output, error_string, &length_of_error_string);
        fprintf(stderr, "MPI Error: %s\n", error_string);
        MPI_Abort(MPI_COMM_WORLD, output);
    }
}

MPI_Datatype MPIUtils::MPI_Vec2{};
MPI_Datatype MPIUtils::MPI_CoordsValue{};

void MPIUtils::init_datatypes() {

    // Create MPI_Vec2 datatype
    MPI_Type_contiguous(2, MPI_UNSIGNED, &MPI_Vec2);
    MPI_Type_commit(&MPI_Vec2);

    // Create MPI_CoordsValue datatype
    int block_lengths[2] = {1, 1};
    MPI_Aint displacements[2] = {0, sizeof(unsigned int) * 2};
    MPI_Datatype types[2] = {MPI_Vec2, MPI_C_BOOL};
    MPI_Type_create_struct(2, block_lengths, displacements, types, &MPI_CoordsValue);
    MPI_Type_commit(&MPI_CoordsValue);
}


int MPIUtils::get_rank() {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    return rank;
}

int MPIUtils::get_size() {
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    return size;
}


void MPIUtils::recv_request(
        vector<MPI_Request> &recv_requests,
        vector<CoordsValue *> &recv_data
) {
    auto *c = new CoordsValue{};
    MPI_Request req = 0;
    int irecv = MPI_Irecv(c, 1, MPIUtils::MPI_CoordsValue, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &req);
    validateMPIOutput(irecv);
    recv_requests.push_back(req);
    recv_data.push_back(c);
}

void MPIUtils::send_request(
        vector<MPI_Request> &send_requests,
        vector<CoordsValue *> &send_data,
        unsigned int dest,
        CoordsValue *cv
) {
    MPI_Request req = 0;
    int ierr = MPI_Isend(cv, 1, MPIUtils::MPI_CoordsValue, (int) dest, 0, MPI_COMM_WORLD, &req);
    validateMPIOutput(ierr);
    send_requests.push_back(req);
    send_data.push_back(cv);
}
