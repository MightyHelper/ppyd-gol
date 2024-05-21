#include <mpi.h>
#include <iostream>
#include <sstream>
#include "canvas.hxx"

using namespace std;

void plot_topo();

void create_topo();

void init_relative();

void debug_neighbors();

void debug_ranks();

bool comunicate(int o, int i, int dst, int src);

int world_size;
int world_rank;
int cart_coords[2];
MPI_Comm comm2d;
int dims[2] = {0, 0};
int periods[2] = {1, 1};
int relative[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
Canvas<10, 10> canvas{};

void comunicate(){
	// Communicate top canvas row to node above
	for (int o = 1; o != 9; o = 9) {
		for (int i = 1; i < 9; i++) {
			// 0 1 2
			// 3 4 5
			// 6 7 8
			canvas.at(10 - o, i) = comunicate(o, i, 1, 7);
			canvas.at(i, 10 - o) = comunicate(o, i, 3, 5);
		}
	}
	// Communicate corners
	canvas.at(1, 1) = comunicate(9, 9, 2, 6);
	canvas.at(9, 9) = comunicate(1, 1, 6, 2);
	canvas.at(1, 9) = comunicate(9, 1, 0, 8);
	canvas.at(9, 1) = comunicate(1, 9, 8, 0);
}

bool comunicate(int o, int i, int dst, int src) {
	bool send = canvas.at(i, o);
	bool recv;
	MPI_Sendrecv(
		&send, 1, MPI_CXX_BOOL, relative[dst], 0,
		&recv, 1, MPI_CXX_BOOL, relative[src], 0,
		comm2d, MPI_STATUS_IGNORE
	);
	return recv;
}

int main() {
	MPI_Init(nullptr, nullptr);
	create_topo();
	init_relative();
	debug_ranks();
	canvas.init();
	comunicate();



	MPI_Finalize();
	return 0;
}

void debug_ranks() {
	int all_ranks[world_size];
	MPI_Allgather(&world_rank, 1, MPI_INT, all_ranks, 1, MPI_INT, MPI_COMM_WORLD);
	if (world_rank == 0) {
		for (int i = 0; i < world_size; i++) {
			cout << all_ranks[i] << " ";
			if ((i + 1) % dims[0] == 0) cout << endl;
		}
		cout << endl;
	}
}

void create_topo() {
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Dims_create(world_size, 2, dims);
	MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &comm2d);
	MPI_Cart_coords(comm2d, world_rank, 2, cart_coords);

}
void init_relative(){
	// https://stackoverflow.com/questions/20813185/what-are-source-and-destination-parameters-in-mpi-cart-shift
	relative[4] = world_rank;
	MPI_Cart_shift(comm2d, 0, 1,  &relative[1], &relative[7]);
	MPI_Cart_shift(comm2d, 1, 1,  &relative[3], &relative[5]);
	// Get diagonals
	// 0 1 2
	// 3 4 5
	// 6 7 8
	MPI_Sendrecv(
		&relative[1], 1, MPI_INT, relative[5], 0,
		&relative[0], 1, MPI_INT, relative[3], 0,
		comm2d, MPI_STATUS_IGNORE
	);
	MPI_Sendrecv(
		&relative[1], 1, MPI_INT, relative[3], 0,
		&relative[2], 1, MPI_INT, relative[5], 0,
		comm2d, MPI_STATUS_IGNORE
	);
	MPI_Sendrecv(
		&relative[7], 1, MPI_INT, relative[5], 0,
		&relative[6], 1, MPI_INT, relative[3], 0,
		comm2d, MPI_STATUS_IGNORE
	);
	MPI_Sendrecv(
		&relative[7], 1, MPI_INT, relative[3], 0,
		&relative[8], 1, MPI_INT, relative[5], 0,
		comm2d, MPI_STATUS_IGNORE
	);
}

void debug_neighbors() {
	MPI_Barrier(comm2d);
	stringstream x;
	x << "Rank: " << world_rank << " Coords: " << cart_coords[0] << " " << cart_coords[1] << '\n';
	for (int i = 0; i < 9; i++) {
		x << relative[i] << " ";
		if ((i + 1) % 3 == 0) x << '\n';
	}
	x << '\n';
	cout << x.str();
}

void plot_topo() {
	if (world_rank == 0) cout << "\033[2J" << "\033[0;0H" << flush;
	MPI_Barrier(MPI_COMM_WORLD);
	cout << "\033[" << cart_coords[0] + 1 << ";" << cart_coords[1] + 1 << "H" << "x";
	MPI_Barrier(MPI_COMM_WORLD);
	cout << "\n\n" << flush;
}

