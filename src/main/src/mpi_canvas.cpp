#include "../include/mpi_canvas.h"
#include "../include/mpi_utils.h"
#include "../include/state_loader.h"

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

void MPICanvas::load_file(const std::string &str, int dx, int dy) {
  StateLoader::load_from_file(str.c_str(), dx, dy, [this](unsigned int x, unsigned int y, bool value) {
    Vec2<unsigned int> global{x, y};
    try {
      Vec2<unsigned int> local = global.global_to_local_memoized(cart_coords, item_size, dims);
      canvas->at(local.x, local.y) = value;
    } catch (const std::exception &e) {
      // Pass
    }
  });
}


void MPICanvas::comunicate() {
  // 0 1 2 <- [ Old scheme ]    0 3 6 | [ Conversion table ]
  // 3 4 5    [ New scheme ] -> 1 4 7 | [0, 4, 8] the same
  // 6 7 8                      2 5 8 | [1 <=> 3] [2 <=> 6] [5 <=> 7]
  array<vector<CoordsValue>, 9> send_data{};
  array<vector<CoordsValue>, 9> recv_data{};
  for (unsigned int o = 1; o < item_size.y + 1; o++) {
    Vec2 c1{(unsigned int) 1, o};
    Vec2 c2{(unsigned int) item_size.x, o};
    Vec2 c3{o, (unsigned int) 1};
    Vec2 c4{o, (unsigned int) item_size.x};
    Vec2 g1 = c1.to_global(cart_coords, item_size, dims);
    Vec2 g2 = c2.to_global(cart_coords, item_size, dims);
    Vec2 g3 = c3.to_global(cart_coords, item_size, dims);
    Vec2 g4 = c4.to_global(cart_coords, item_size, dims);
    send_data[3].push_back(CoordsValue{g1, canvas->at(c1.x, c1.y)});
    send_data[5].push_back(CoordsValue{g2, canvas->at(c2.x, c2.y)});
    send_data[1].push_back(CoordsValue{g3, canvas->at(c3.x, c3.y)});
    send_data[7].push_back(CoordsValue{g4, canvas->at(c4.x, c4.y)});
  }
  send_data[0].push_back(CoordsValue{Vec2{(unsigned int)1,(unsigned int) 1}.to_global(cart_coords, item_size, dims), canvas->at(1, 1)});
  send_data[2].push_back(CoordsValue{Vec2{item_size.x, (unsigned int)1}.to_global(cart_coords, item_size, dims), canvas->at(item_size.x, 1)});
  send_data[6].push_back(CoordsValue{Vec2{(unsigned int)1, item_size.y}.to_global(cart_coords, item_size, dims), canvas->at(1, item_size.y)});
  send_data[8].push_back(CoordsValue{Vec2{item_size.x, item_size.y}.to_global(cart_coords, item_size, dims), canvas->at(item_size.x, item_size.y)});

  // Send packed
  vector<MPI_Request> send_requests;
  for (unsigned int i = 0; i < 9; i++) {
    if (relative[i] == relative[4]) continue;
    MPI_Isend(
     send_data[i].data(), (int) send_data[i].size(), MPIUtils::MPI_CoordsValue,
     (int) relative[i], 0, comm, &send_requests.emplace_back()
    );
  }
  // Recv packed
  vector<MPI_Request> recv_requests;
  for (unsigned int i = 0; i < 9; i++) {
    if (relative[i] == relative[4]){
      continue;
    }
    recv_data[i].resize(send_data[i].size());
    MPI_Irecv(
     recv_data[i].data(), (int) recv_data[i].size(), MPIUtils::MPI_CoordsValue,
     (int) relative[i], 0, comm, &recv_requests.emplace_back()
    );
  }

  MPI_Waitall((int) recv_requests.size(), recv_requests.data(), MPI_STATUSES_IGNORE);
  for (unsigned int i = 0; i < 9; i++) {
    for (auto &j: relative[i] == relative[4] ? send_data[i] : recv_data[i]) {
      Vec2 local = j.pos.global_to_local_memoized(cart_coords, item_size, dims);
      canvas->at(local.x, local.y) = j.value;
    }
  }
  MPI_Waitall((int) send_requests.size(), send_requests.data(), MPI_STATUSES_IGNORE);
}

void MPICanvas::iter() {
  comunicate();
  MPI_Barrier(MPI_COMM_WORLD);
  canvas->iter();
}
