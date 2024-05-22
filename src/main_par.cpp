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

int world_size;
int world_rank;
int cart_coords[2];
MPI_Comm comm2d;
int dims[2] = {0, 0};
int periods[2] = {1, 1};
int relative[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
const int width = 8;
const int height = 8;
#define ANALYSIS_RANK 4
Canvas<width + 2, height + 2> canvas{};

int positive_modulo(int i, int n) {
    return (i % n + n) % n;
}

struct Coords {
    int x;
    int y;

    Coords left(){ return Coords{x + width, y}; }
    Coords right(){ return Coords{x - width, y}; }
    Coords up(){ return Coords{x, y + height}; }
    Coords down(){ return Coords{x, y - height}; }
    Coords up_right(){ return Coords{x - width, y + height}; }
    Coords up_left(){ return Coords{x + width, y + height}; }
    Coords down_right(){ return Coords{x - width, y - height}; }
    Coords down_left(){ return Coords{x + width, y - height}; }
    friend ostream &operator<<(ostream &os, const Coords &c) {
        return os << "Coords{" << c.x << "," << c.y << "}";
    }
};

void comunicate(const Coords &get_data_from, const Coords &put_data_in, int send_data_to, int receive_data_from) {
    bool send_data = canvas.at(get_data_from.x, get_data_from.y);
    bool recv_data;
    MPI_Sendrecv(
            &send_data, 1, MPI_CXX_BOOL, relative[send_data_to], 0,
            &recv_data, 1, MPI_CXX_BOOL, relative[receive_data_from], 0,
            comm2d, MPI_STATUS_IGNORE
    );
    canvas.at(put_data_in.x, put_data_in.y) = recv_data;
}

void comunicate() {
    // 0 1 2
    // 3 4 5
    // 6 7 8
    // Communicate top canvas row to node above
    Coords node{cart_coords[0], cart_coords[1]};
    for (int i = 1; i < 9; i++) {
        Coords top_row{i, 1};
        Coords bottom_row{i, 8};
        Coords right_column{1, i};
        Coords left_column{8, i};
        comunicate(top_row, top_row.up(), 1, 7);
        comunicate(bottom_row, bottom_row.down(), 7, 1);
        comunicate(right_column, right_column.right(), 5, 3);
        comunicate(left_column, left_column.left(), 3, 5);
//        if (world_rank == ANALYSIS_RANK) cout << top_row << " => " << up << endl;
//        if (world_rank == ANALYSIS_RANK) cout << bottom_row << " => " << down << endl;
//        if (world_rank == ANALYSIS_RANK) cout << right_column << " => " << right << endl;
//        if (world_rank == ANALYSIS_RANK) cout << left_column << " => " << left << endl;
    }
    Coords bottom_right{1, 1};
    Coords top_left{8, 8};
    Coords top_right{1, 8};
    Coords bottom_left{8, 1};
    comunicate(bottom_right, bottom_right.down_right(), 0, 8);
    comunicate(top_left, top_left.up_left(), 8, 0);
    comunicate(top_right, top_right.up_right(), 6, 2);
    comunicate(bottom_left, bottom_left.down_left(), 2, 6);
}


int main() {
    MPI_Init(nullptr, nullptr);
    create_topo();
    init_relative();
    debug_ranks();
    if (world_rank == ANALYSIS_RANK)
        debug_neighbors();
    canvas.init();
    if (world_rank != ANALYSIS_RANK) {
        for (int i = 0; i < 10; i++) {
            for (int o = 0; o < 10; o++) {
                canvas.at(i, o) = true;
            }
        }
    }
    comunicate();
    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 0) {
        cout << world_rank << endl;
        canvas.print();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 3){
        cout << world_rank << endl;
        canvas.print();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 6){
        cout << world_rank << endl;
        canvas.print();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 4){
        cout << world_rank << endl;
        canvas.print();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 2) {
        cout << world_rank << endl;
        canvas.print();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 5){
        cout << world_rank << endl;
        canvas.print();
    }
    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 8){
        cout << world_rank << endl;
        canvas.print();
    }
    MPI_Barrier(MPI_COMM_WORLD);


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

void init_relative() {
    // https://stackoverflow.com/questions/20813185/what-are-source-and-destination-parameters-in-mpi-cart-shift
    relative[4] = world_rank;
    MPI_Cart_shift(comm2d, 0, 1, &relative[1], &relative[7]);
    MPI_Cart_shift(comm2d, 1, 1, &relative[3], &relative[5]);
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

