#include <mpi.h>
#include <iostream>
#include <sstream>
#include <atomic>
#include <chrono>
#include <stddef.h>
#include "canvas.hxx"

using namespace std;

void plot_topo();

void create_topo();

void init_relative();

void debug_neighbors();

void debug_ranks();

void print_all();

void print_all2();

void print_all3();

#define rgb(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m"
// List of rgb ansi colors
const char *ansi_colors[] = {
        rgb(255, 0, 0),
        rgb(0, 255, 0),
        rgb(100, 100, 255),
        rgb(255, 255, 0),
        rgb(0, 255, 255),
        rgb(255, 0, 255),
        rgb(255, 255, 255),
        rgb(128, 255, 128),
        rgb(0, 128, 255),
        rgb(255, 128, 128),
        rgb(128, 0, 0),
        rgb(128, 128, 0),
        rgb(0, 128, 0),
        rgb(128, 0, 128),
        rgb(0, 128, 128),
        rgb(0, 255, 128),
        rgb(128, 128, 255),
};
int world_size;
int world_rank;
int cart_coords[2];
MPI_Comm comm2d;
int dims[2] = {0, 0};
int periods[2] = {1, 1};
int relative[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
const int width = 3;
const int height = 3;
#define ANALYSIS_RANK 0
Canvas<width + 2, height + 2> canvas{};

struct Coords {
    int x;
    int y;

    /*[[nodiscard]] Coords global_to_local() const {
        return Coords{positive_modulo(x - cart_coords[0] * width - 1, width + 2), positive_modulo(y - cart_coords[1] * height - 1, height + 2)};
    }*/
    [[nodiscard]] Coords global_to_local() const {
        for (int i = 0; i < width + 2; i++) {
            for (int o = 0; o < height + 2; o++) {
                const Coords g = Coords{i, o}.to_global();
                if (g.x == x && g.y == y) {
                    return Coords{i, o};
                }
            }
        }
        stringstream ss;
        ss << "Coords{" << x << "," << y << "} not found in global_to_local for rank " << world_rank;
//        if (ANALYSIS_RANK == world_rank)
        cout << ss.str() << endl;
//        throw std::runtime_error(ss.str());
        return Coords{0, 0};
    }


    [[nodiscard]] Coords to_global() const {
        return Coords{positive_modulo(x + cart_coords[0] * width - 1, width * dims[0]),
                      positive_modulo(y + cart_coords[1] * height - 1, height * dims[1])};
    }

    [[nodiscard]] Coords to_super_global() const {
        return Coords{x + cart_coords[0] * (width + 2), y + cart_coords[1] * (height + 2)};
    }

    [[nodiscard]] long long to_global_tag() const {
        Coords g = to_global();
        return wrapped_coords(width * dims[0], height * dims[1], g.x, g.y);
    }

    [[nodiscard]] long long to_super_global_tag() const {
        Coords g = to_super_global();
        return wrapped_coords((width + 2) * dims[0], (height + 2) * dims[1], g.x, g.y);
    }

    friend ostream &operator<<(ostream &os, const Coords &c) {
        return os << "Coords{" << c.x << "," << c.y << "}";
    }
};

struct CoordsValue {
    Coords pos;
    bool value;
};

MPI_Datatype MPI_CoordsValue;
MPI_Datatype MPI_Coords;

//#define validateMPIoutput(value) cout << "Validate " << __LINE__ << " " << __FUNCTION__ << endl; _validateMPIoutput(value)
#define validateMPIoutput(value) _validateMPIoutput(value)

void _validateMPIoutput(int output) {
    if (output != MPI_SUCCESS) {
        cout << "Error: " << output << endl;
        char error_string[MPI_MAX_ERROR_STRING];
        int length_of_error_string;
        MPI_Error_string(output, error_string, &length_of_error_string);
        fprintf(stderr, "MPI Error: %s\n", error_string);
        MPI_Abort(MPI_COMM_WORLD, output);
    }
}

void init_datatypes() {
    validateMPIoutput(MPI_Type_contiguous(2, MPI_INT, &MPI_Coords));
    validateMPIoutput(MPI_Type_commit(&MPI_Coords));
    // Create a struct
    int block_lengths[2] = {2, 1};
    MPI_Aint displacements[2] = {offsetof(CoordsValue, pos), offsetof(CoordsValue, value)};
    MPI_Datatype types[2] = {MPI_Coords, MPI_CXX_BOOL};
    validateMPIoutput(MPI_Type_create_struct(2, block_lengths, displacements, types, &MPI_CoordsValue));
    validateMPIoutput(MPI_Type_commit(&MPI_CoordsValue));
}


void send_request(vector<MPI_Request *> &send_requests, int dest, CoordsValue cv);

void dsend_request(vector<MPI_Request *> &send_requests, int dest, CoordsValue cv);

void recv_request(vector<MPI_Request *> &recv_requests, vector<CoordsValue *> &recv_data);

void _simple_test();

void comunicate() {
    // 0 1 2 <- [ Old scheme ]    0 3 6 | [ Conversion table ]
    // 3 4 5    [ New scheme ] -> 1 4 7 | [0, 4, 8] the same
    // 6 7 8                      2 5 8 | [1 <=> 3] [2 <=> 6] [5 <=> 7]
    vector<MPI_Request *> send_requests;
    vector<MPI_Request *> recv_requests;
    vector<CoordsValue *> recv_data;

    for (int i = 1; i < width + 1; i++) {
        for (int o = 1; o < height + 1; o++) {
            Coords c{i, o};
            bool send = canvas.at(c.x, c.y);
            Coords g = c.to_global();
            if (c.x == 1) send_request(send_requests, relative[3], CoordsValue{g, send});
            if (c.x == width) send_request(send_requests, relative[5], CoordsValue{g, send});
            if (c.y == 1) send_request(send_requests, relative[1], CoordsValue{g, send});
            if (c.y == height) send_request(send_requests, relative[7], CoordsValue{g, send});
            if (c.x == 1 && c.y == 1) send_request(send_requests, relative[0], CoordsValue{g, send});
            if (c.x == 1 && c.y == height) send_request(send_requests, relative[6], CoordsValue{g, send});
            if (c.x == width && c.y == 1) send_request(send_requests, relative[2], CoordsValue{g, send});
            if (c.x == width && c.y == height) send_request(send_requests, relative[8], CoordsValue{g, send});
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    for (int i = 0; i < width + 2; i++) {
        for (int o = 0; o < height + 2; o++) {
            Coords c{i, o};
            if (c.x != 0 && c.y != 0 && c.x != width + 1 && c.y != height + 1) continue;
            recv_request(recv_requests, recv_data);
        }
    }
    MPI_Request reqs[recv_requests.size()];
    for (int i = 0; i < recv_requests.size(); i++) reqs[i] = *recv_requests[i];
    MPI_Waitall((int) recv_requests.size(), reqs, MPI_STATUSES_IGNORE);
    for (int i = 0; i < recv_requests.size(); i++) {
//        if (recv_data[i]->pos.x == -1 and recv_data[i]->pos.y == -1) {
//            continue;
//        }
        Coords local = recv_data[i]->pos.global_to_local();
        canvas.at(local.x, local.y) = recv_data[i]->value;
        if (canvas.at(local.x, local.y))
            cout << "Rank: " << world_rank << " Recv: " << recv_data[i]->pos << "=>" << local << " "
                 << canvas.at(local.x, local.y) << endl;
//        if (ANALYSIS_RANK == world_rank) {
            cout << "Recv[" << world_rank << "] " << recv_data[i]->pos << "=>" << local << " " << recv_data[i]->value << canvas.at(local.x, local.y)
                 << endl;
//        }
    }
    MPI_Request reqs2[send_requests.size()];
    for (int i = 0; i < send_requests.size(); i++) reqs2[i] = *send_requests[i];
    MPI_Waitall((int) send_requests.size(), reqs2, MPI_STATUSES_IGNORE);
    for (auto &i: recv_data) delete i;
    for (auto &i: recv_requests) delete i;
}


void recv_request(vector<MPI_Request *> &recv_requests, vector<CoordsValue *> &recv_data) {
    auto *cv = new CoordsValue;
    auto req = new MPI_Request;
    int irecv = MPI_Irecv(cv, 1, MPI_CoordsValue, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, req);
    validateMPIoutput(irecv);
    recv_requests.push_back(req);
    recv_data.push_back(cv);
}

void send_request(vector<MPI_Request *> &send_requests, int dest, CoordsValue cv) {
    auto *req = new MPI_Request;
    if (dest == ANALYSIS_RANK) {
        cout << "Send[" << ANALYSIS_RANK << "] (" << world_rank << ") " << cv.pos << " " << cv.value << endl;
    }
    int ierr = MPI_Isend(&cv, 1, MPI_CoordsValue, dest, 0, MPI_COMM_WORLD, req);
    validateMPIoutput(ierr);
    send_requests.push_back(req);
}

void dsend_request(vector<MPI_Request *> &send_requests, int dest, CoordsValue cv) {
    auto *req = new MPI_Request;
//    if (dest == ANALYSIS_RANK){
//        cout << "Send[" << ANALYSIS_RANK << "] (" << world_rank << ") " << cv.pos << " " << cv.value << endl;
//    }
    cv.pos = Coords{-1, -1};
    int ierr = MPI_Isend(&cv, 1, MPI_CoordsValue, dest, 0, MPI_COMM_WORLD, req);
    validateMPIoutput(ierr);
    send_requests.push_back(req);
}


int main() {
    MPI_Init(nullptr, nullptr);
    init_datatypes();
    create_topo();
    init_relative();
//    _simple_test();
    canvas.init();
    if (world_rank == 4) {
        canvas.at(width, height) = true;
    }
    comunicate();
    MPI_Barrier(MPI_COMM_WORLD);
    usleep(5000);
    cout << "\n\n\n" << endl;
    usleep(5000);
    MPI_Barrier(MPI_COMM_WORLD);
    print_all3();
    debug_neighbors();
    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}


void _simple_test() {
    if (world_rank == 0) {
        MPI_Request reqs2[10]{};
        for (int i = 0; i < 10; i++) {
            auto req = new MPI_Request;
            int can = 2;
            validateMPIoutput(MPI_Isend(&can, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, req));
            reqs2[i] = (*req);
        }
        validateMPIoutput(MPI_Waitall(10, reqs2, MPI_STATUSES_IGNORE));
    }
    if (world_rank == 1) {
        MPI_Request reqs[10]{};
        for (int i = 0; i < 10; i++) {
            int can2 = 0;
            auto req2 = new MPI_Request;
            validateMPIoutput(MPI_Irecv(&can2, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, req2));
            reqs[i] = *req2;
        }
        validateMPIoutput(MPI_Waitall(10, reqs, MPI_STATUSES_IGNORE));
    }
}

void print_all() {
    for (int i = 0; i < 10; i++) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (world_rank == i) {
            cout << world_rank << endl;
            canvas.print();
        }
        usleep(10000);
    }
}

void print_all2() {
    const int dx = 3;
    const int dy = 4;
    Canvas<width * dx, height * dy> canvas2{};
    canvas2.init();
    for (int i = 0; i < width; i++) {
        for (int o = 0; o < height; o++) {
            Coords c{i + 1, o + 1};
            Coords g = c.to_global();
            cout << "\033[" << (g.x * 2) + 1 << ";" << (g.y * 4) + 1 << "H" << "\033[3" << world_rank + 1 << "m"
                 << c.to_global_tag() << hex << /*faint*/ "\033[2m" <<
                 world_rank << dec << "\033[0m";
//			     << (canvas.at(c.x + 1, c.y + 1) ? 'X' : '.') << " ";
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    cout << "\033[0m\033[" << height * dy + 1 << ";1H" << flush;
}

void print_all3() {
    const int dx = 3;
    const int dy = 4;
    Canvas<width * dx, height * dy> canvas2{};
    canvas2.init();
    for (int i = 0; i < width + 2; i++) {
        for (int o = 0; o < height + 2; o++) {
            Coords c{i, o};
            Coords g = c.to_super_global();
            Coords g2 = c.to_global();
            bool send = canvas.at(c.x, c.y);
            const char *bg = i > 0 && i < width + 1 && o > 0 && o < height + 1 ? "\033[48;2;20;40;20m" : "\033[40m";

            const char *underline = send ? "\033[4m" : "";
            cout << bg << underline << "\033[" << (g.y) + 27 << ";" << (g.x * 10) + 1 << "H" << ansi_colors[world_rank]
                 //                 << (canvas.at(c.x, c.y) ? 'X' : '.') << hex << /*faint*/ "\033[2m" <<
                 << g2.x << "|" << g2.y << /*faint*/ "\033[2m" <<
                 //                 c.x << "|" << c.y << "\033[0;3m" << world_rank << "\033[0m";
                 c.x << "|" << c.y << "\033[0m" << underline << bg << world_rank << "\033[0m" << flush;
            MPI_Barrier(MPI_COMM_WORLD);
            usleep(5000 * world_rank);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    usleep(500);
    MPI_Barrier(MPI_COMM_WORLD);
    cout << "\033[0m\033[" << (height + 2) * (dy * 2) + 25 << ";1H" << flush;
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
    MPI_Cart_shift(comm2d, 0, 1, &relative[3], &relative[5]);
    MPI_Cart_shift(comm2d, 1, 1, &relative[1], &relative[7]);
    // 0 1 2 <- [ Old scheme ]    0 3 6 | [ Conversion table ]
    // 3 4 5    [ New scheme ] -> 1 4 7 | [0, 4, 8] the same
    // 6 7 8                      2 5 8 | [1 <=> 3] [2 <=> 6] [5 <=> 7]
    MPI_Sendrecv(
            &relative[3], 1, MPI_INT, relative[7], 0,
            &relative[0], 1, MPI_INT, relative[1], 0,
            comm2d, MPI_STATUS_IGNORE
    );
    MPI_Sendrecv(
            &relative[3], 1, MPI_INT, relative[1], 0,
            &relative[6], 1, MPI_INT, relative[7], 0,
            comm2d, MPI_STATUS_IGNORE
    );
    MPI_Sendrecv(
            &relative[5], 1, MPI_INT, relative[7], 0,
            &relative[2], 1, MPI_INT, relative[1], 0,
            comm2d, MPI_STATUS_IGNORE
    );
    MPI_Sendrecv(
            &relative[5], 1, MPI_INT, relative[1], 0,
            &relative[8], 1, MPI_INT, relative[7], 0,
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

