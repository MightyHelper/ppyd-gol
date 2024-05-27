#include "../include/mpi_canvas.h"
#include "../include/mpi_utils.h"
using namespace std;
MPICanvas::MPICanvas(
        Canvas *canvas,
        Vec2<unsigned int> item_size,
        Vec2<unsigned int> periods
) : canvas(canvas), comm(), item_size(item_size), periods(periods) {
    this->rank = MPIUtils::get_rank();
    this->size = MPIUtils::get_size();
    init_topology();
    this->relative = init_relative();
}

void MPICanvas::init_topology() {
    Vec2<int> temp_dims{(int) dims.x, (int) dims.y};
    const Vec2<int> temp_periods{(int) periods.x, (int) periods.y};
    Vec2<int> temp_cart_coords{(int) cart_coords.x, (int) cart_coords.y};
    MPI_Dims_create((int) size, 2, temp_dims.array());
    dims.x = (unsigned int) temp_dims.x;
    dims.y = (unsigned int) temp_dims.y;
    MPI_Cart_create(MPI_COMM_WORLD, 2, temp_dims.array(), temp_periods.array(), 1, &comm);
    MPI_Cart_coords(comm, (int) rank, 2, temp_cart_coords.array());
    cart_coords.x = (unsigned int) temp_cart_coords.x;
    cart_coords.y = (unsigned int) temp_cart_coords.y;
}


[[nodiscard]] array<unsigned int, 9> MPICanvas::init_relative() const {
    // https://stackoverflow.com/questions/20813185/what-are-source-and-destination-parameters-in-mpi-cart-shift
    array<int, 9> out{};
    out[4] = (int) rank;
    MPI_Cart_shift(comm, 0, 1, &out[3], &out[5]);
    MPI_Cart_shift(comm, 1, 1, &out[1], &out[7]);
    // 0 1 2 <- [ Old scheme ]    0 3 6 | [ Conversion table ]
    // 3 4 5    [ New scheme ] -> 1 4 7 | [0, 4, 8] the same
    // 6 7 8                      2 5 8 | [1 <=> 3] [2 <=> 6] [5 <=> 7]
    MPI_Sendrecv(
            &out[3], 1, MPI_INT, out[7], 0,
            &out[0], 1, MPI_INT, out[1], 0,
            comm, MPI_STATUS_IGNORE
    );
    MPI_Sendrecv(
            &out[3], 1, MPI_INT, out[1], 0,
            &out[6], 1, MPI_INT, out[7], 0,
            comm, MPI_STATUS_IGNORE
    );
    MPI_Sendrecv(
            &out[5], 1, MPI_INT, out[7], 0,
            &out[2], 1, MPI_INT, out[1], 0,
            comm, MPI_STATUS_IGNORE
    );
    MPI_Sendrecv(
            &out[5], 1, MPI_INT, out[1], 0,
            &out[8], 1, MPI_INT, out[7], 0,
            comm, MPI_STATUS_IGNORE
    );
    array<unsigned int, 9> out2{};
    for (int i = 0; i < 9; i++) out2[i] = (unsigned int) out[i];
    return out2;
}

void MPICanvas::comunicate() {
    // 0 1 2 <- [ Old scheme ]    0 3 6 | [ Conversion table ]
    // 3 4 5    [ New scheme ] -> 1 4 7 | [0, 4, 8] the same
    // 6 7 8                      2 5 8 | [1 <=> 3] [2 <=> 6] [5 <=> 7]
    vector <MPI_Request> send_requests{};
    vector <MPI_Request> recv_requests{};
    vector < CoordsValue * > send_data{};
    vector < CoordsValue * > recv_data{};

    send_requests.reserve((item_size.x + item_size.y + 2) * 2);
    recv_requests.reserve((item_size.x + item_size.y + 2) * 2);
    send_data.reserve((item_size.x + item_size.y + 2) * 2);
    recv_data.reserve((item_size.x + item_size.y + 2) * 2);

    for (unsigned int i = 1; i < item_size.x + 1; i++) {
        for (unsigned int o = 1; o < item_size.y + 1; o++) {
            Vec2 c{i, o};
            bool send = canvas->at(c.x, c.y);
            Vec2 g = c.to_global(cart_coords, item_size, dims);
            if (c.x == 1) MPIUtils::send_request(send_requests, send_data, relative[3], new CoordsValue{g, send});
            if (c.x == item_size.x) MPIUtils::send_request(send_requests, send_data, relative[5], new CoordsValue{g, send});
            if (c.y == 1) MPIUtils::send_request(send_requests, send_data, relative[1], new CoordsValue{g, send});
            if (c.y == item_size.y) MPIUtils::send_request(send_requests, send_data, relative[7], new CoordsValue{g, send});
            if (c.x == 1 && c.y == 1) MPIUtils::send_request(send_requests, send_data, relative[0], new CoordsValue{g, send});
            if (c.x == 1 && c.y == item_size.y)
                MPIUtils::send_request(send_requests, send_data, relative[6], new CoordsValue{g, send});
            if (c.x == item_size.x && c.y == 1)
                MPIUtils::send_request(send_requests, send_data, relative[2], new CoordsValue{g, send});
            if (c.x == item_size.x && c.y == item_size.y)
                MPIUtils::send_request(send_requests, send_data, relative[8], new CoordsValue{g, send});
        }
    }
    for (unsigned int i = 0; i < item_size.x + 2; i++) {
        for (unsigned int o = 0; o < item_size.y + 2; o++) {
            Vec2 c{i, o};
            if (c.x != 0 && c.y != 0 && c.x != item_size.x + 1 && c.y != item_size.y + 1) continue;
            MPIUtils::recv_request(recv_requests, recv_data);
        }
    }
    MPI_Waitall((int) recv_requests.size(), recv_requests.data(), MPI_STATUSES_IGNORE);
    for (unsigned int i = 0; i < recv_requests.size(); i++) {
        Vec2 local = recv_data[i]->pos.global_to_local(cart_coords, item_size, dims);
        canvas->at(local.x, local.y) = recv_data[i]->value;
    }
    MPI_Waitall((int) send_requests.size(), send_requests.data(), MPI_STATUSES_IGNORE);
}