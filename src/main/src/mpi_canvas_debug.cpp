#include "../include/mpi_canvas_debug.h"
#include "../include/mpi_utils.h"
#include "../include/utils.h"

using namespace std;

void Debug::print_all(const MPICanvas &canvas) {
  for (unsigned int i = 0; i < 10; i++) {
    MPI_Barrier(MPI_COMM_WORLD);
    if (canvas.rank == i) {
      cout << canvas.rank << endl;
      canvas.canvas->print();
    }
    usleep(10000);
  }
}


void Debug::print_all_generic(
 const MPICanvas &canvas,
 Vec2<unsigned int> base,
 Vec2<unsigned int> delta,
 const function<void(stringstream &, Vec2<unsigned int>, Vec2<unsigned int>)> &f
) {
  stringstream ss;
  for (unsigned int i = 0; i < canvas.item_size.x; i++) {
    for (unsigned int o = 0; o < canvas.item_size.y; o++) {
      Vec2 c{i + 1, o + 1};
      Vec2 g = c.to_global(canvas.cart_coords, canvas.item_size, canvas.dims);
      ss << "\033[" << (base.y + g.y * delta.y) + 1 << ";" << (base.x + g.x * delta.x) + 1 << "H";
      f(ss, c, g);
      ss << "\033[0m";
    }
  }
  string str = ss.str();
  string out;
  MPIUtils::MPI_Gather_string(str, out, 0, MPI_COMM_WORLD);
  // Print full_message on root
  if (canvas.rank == 0) {
    cout << out << flush;
    cout << "\033[0m\033[" << canvas.item_size.y * canvas.dims.y * delta.y + 1 + base.y << ";1H" << flush;
  }
}

void Debug::print_all_full_generic(
 const MPICanvas &canvas,
 Vec2<unsigned int> base,
 Vec2<unsigned int> delta,
 const function<void(stringstream &, Vec2<unsigned int>, Vec2<unsigned int>)> &f
) {
  stringstream ss;
  for (unsigned int i = 0; i < canvas.item_size.x + 2; i++) {
    for (unsigned int o = 0; o < canvas.item_size.y + 2; o++) {
      Vec2 c{i, o};
      Vec2 g = c.to_super_global(canvas.cart_coords, canvas.item_size);
      ss << "\033[" << (base.y + g.y * delta.y) + 1 << ";" << (base.x + g.x * delta.x) + 1 << "H";
      f(ss, c, g);
      ss << "\033[0m";
    }
  }
  string str = ss.str();
  string out;
  MPIUtils::MPI_Gather_string(str, out, 0, MPI_COMM_WORLD);
  // Print full_message on root
  if (canvas.rank == 0) {
    cout << out << flush;
    cout << "\033[0m\033[" << (canvas.item_size.y + 2) * canvas.dims.y * delta.y + 1 + base.y << ";1H" << flush;
  }
}

void Debug::print_all_full_generic_line(
 const MPICanvas &canvas,
 Vec2<unsigned int> base,
 Vec2<unsigned int> delta,
 const function<void(stringstream &, Vec2<unsigned int>, Vec2<unsigned int>)> &f
) {
  stringstream ss;
  for (unsigned int i = 0; i < canvas.item_size.x + 2; i++) {
    Vec2<unsigned int> c(i, 0);
    Vec2<unsigned int> g = c.to_super_global(canvas.cart_coords, canvas.item_size);
    ss << "\033[" << (base.y + g.y * delta.y) + 1 << ";" << (base.x + g.x * delta.x) + 1 << "H";
    f(ss, c, g);
    ss << "\033[0m";
  }
  string str = ss.str();
  string out;
  MPIUtils::MPI_Gather_string(str, out, 0, MPI_COMM_WORLD);
  // Print full_message on root
  if (canvas.rank == 0) {
    cout << out << flush;
    cout << "\033[0m\033[" << (canvas.item_size.y + 2) * canvas.dims.y * delta.y + 1 + base.y << ";1H" << flush;
  }
}

void Debug::print_all5(const MPICanvas &canvas, Vec2<unsigned int> base) {
  print_all_generic(
   canvas,
   base,
   Vec2<unsigned int>{2, 1},
   [&canvas](stringstream &f, Vec2<unsigned int> c, Vec2<unsigned int> __attribute__((unused))g) -> void {
     f << Utils::ansi_colors(canvas.rank) << (canvas.canvas->at(c.x, c.y) ? "██" : "░░");
   }
  );
}

void Debug::print_all5_full(const MPICanvas &canvas, Vec2<unsigned int> base) {
  print_all_full_generic(
   canvas,
   base,
   Vec2<unsigned int>{2, 1},
   [&canvas](stringstream &f, Vec2<unsigned int> c, Vec2<unsigned int> __attribute__((unused))g) -> void {
     f << Utils::ansi_colors(canvas.rank) << (canvas.canvas->at(c.x, c.y) ? "██" : "░░");
   }
  );
}

void Debug::print_all_full_all(const MPICanvas &canvas, Vec2<unsigned int> base) {
  Debug::print_all_full_generic(
   canvas,
   base,
   Vec2<unsigned int>{12, 2},
   [&canvas](stringstream &f, Vec2<unsigned int> c, Vec2<unsigned int> __attribute__((unused))g) {
     Vec2<unsigned int> g2 = c.to_global(canvas.cart_coords, canvas.item_size, canvas.dims);
     const char *und = canvas.canvas->at(c.x, c.y) ? "\033[4m" : "";
     f << und << Utils::ansi_colors(canvas.rank)
       << c.x << "|" << c.y << /*dim*/ "\033[2m"
       << g.x << "|" << g.y << "\033[0m" << und << Utils::ansi_colors(canvas.rank) << "\033[1m"
       << g2.x << "|" << g2.y;
   }
  );
}

void Debug::debug_ranks(const MPICanvas &canvas) {
  vector<unsigned int> all_ranks(canvas.size);
  MPI_Allgather(&canvas.rank, 1, MPI_UNSIGNED, all_ranks.data(), 1, MPI_UNSIGNED, MPI_COMM_WORLD);
  if (canvas.rank == 0) {
    for (unsigned int i = 0; i < canvas.size; i++) {
      cout << all_ranks[i] << " ";
      if ((i + 1) % canvas.dims.x == 0) cout << endl;
    }
    cout << endl;
  }
}

void Debug::debug_neighbors(const MPICanvas &canvas) {
  stringstream x;
  x << "Rank: " << canvas.rank << " Vec2: " << canvas.cart_coords.x << " " << canvas.cart_coords.y << '\n';
  for (int i = 0; i < 9; i++) {
    x << canvas.relative[i] << " ";
    if ((i + 1) % 3 == 0) x << '\n';
  }
  x << '\n';
  cout << x.str();
}

void Debug::plot_topo(const MPICanvas &canvas) {
  if (canvas.rank == 0) cout << "\033[2J" << "\033[0;0H" << flush;
  MPI_Barrier(MPI_COMM_WORLD);
  cout << "\033[" << canvas.cart_coords.x + 1 << ";" << canvas.cart_coords.y + 1 << "H" << "x";
  MPI_Barrier(MPI_COMM_WORLD);
  cout << "\n\n" << flush;
}

void Debug::debug_mpi_datatypes(const MPICanvas &canvas) {
  const unsigned int root = 0;
  vector<MPI_Request> requests{};
  vector<CoordsValue *> sends{};
  if (canvas.rank == root) {
    for (unsigned int i = 0; i < canvas.size; i++) {
      MPIUtils::send_request(requests, sends, i,
                              new CoordsValue{Vec2<unsigned int>{root + 69, root + 42}, true});
    }
  }
  vector<MPI_Request> responses{};
  vector<CoordsValue *> recvs{};
  MPIUtils::recv_request(responses, recvs);
  MPI_Waitall((int) responses.size(), responses.data(), MPI_STATUSES_IGNORE);
  for (auto &i: recvs) {
    cout << "Rank: " << canvas.rank << " Pos: " << i->pos << " Value: " << i->value << endl;
  }
  if (canvas.rank == root) {
    MPI_Waitall((int) requests.size(), requests.data(), MPI_STATUSES_IGNORE);
  }
  for (auto &i: recvs) delete i;
  for (auto &i: sends) delete i;
  // Expect to see 3, 3; 3, 3...
}
