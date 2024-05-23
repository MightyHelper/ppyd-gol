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


int world_size;
int world_rank;
int cart_coords[2];
MPI_Comm comm2d;
int dims[2] = {0, 0};
int periods[2] = {1, 1};
int relative[9] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
const int width = 3;
const int height = 3;
#define ANALYSIS_RANK 4
Canvas<width + 2, height + 2> canvas{};

struct Coords {
		int x;
		int y;

		[[nodiscard]] Coords to_global() const {
			return Coords{x + cart_coords[0] * width, y + cart_coords[1] * height};
		}

		[[nodiscard]] Coords to_super_global() const {
			return Coords{x + cart_coords[0] * (width + 2), y + cart_coords[1] * (height + 2)};
		}

		[[nodiscard]] long long to_global_tag() const {
			Coords g = to_global();
			return wrapped_coords(width * dims[0], height * dims[1], g.x - 1, g.y - 1);
		}

		[[nodiscard]] long long to_super_global_tag() const {
			Coords g = to_super_global();
			return wrapped_coords((width + 2) * dims[0], (height + 2) * dims[1], g.x, g.y);
		}

		friend ostream &operator<<(ostream &os, const Coords &c) {
			return os << "Coords{" << c.x << "," << c.y << "}";
		}
};

struct CoordsValue{
		Coords pos;
		bool value;
};

MPI_Datatype MPI_CoordsValue;
MPI_Datatype MPI_Coords;

void validateMPIoutput(int output){
	if (output != MPI_SUCCESS) {
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


void send_request(vector<MPI_Request> &send_requests, int dest, CoordsValue cv);

void recv_request(vector<MPI_Request> &recv_requests, vector<CoordsValue *> &recv_data);

void MPI_Manual_Waitall(int count, MPI_Request *requests, MPI_Status *statuses) {
	// Wait for each using waitAny
	int done = 0;
	while (done < count) {
		int index;
		cout << "(" << world_rank << ") " << "Waiting for any " << done + 1 << "/" << count << endl;
		MPI_Status status;
		int err = MPI_Waitany(count, requests, &index, &status);
		if (err != MPI_SUCCESS) {
			char error_string[MPI_MAX_ERROR_STRING];
			int length_of_error_string;
			MPI_Error_string(err, error_string, &length_of_error_string);
			printf("MPI_Waitany error: %s\n", error_string);
			// Handle the error appropriately, e.g., abort
			MPI_Abort(MPI_COMM_WORLD, err);
		}
		int error_code = status.MPI_ERROR;
		if (error_code != MPI_SUCCESS) {
			char error_string[MPI_MAX_ERROR_STRING];
			int length_of_error_string;
			MPI_Error_string(error_code, error_string, &length_of_error_string);
			printf("MPI Error: %s\n", error_string);
		}

		cout << "Done [" << index << "] " << "src:" << status.MPI_SOURCE << " tag:" << status.MPI_TAG << " error:" << status.MPI_ERROR << endl;
		done++;
	}
}

void comunicate() {
	// 0 1 2
	// 3 4 5
	// 6 7 8
	// Iterate over the border and post an async send with tag [global_x, global_y]
	// Then receive the data and update the buffer
	vector<MPI_Request> send_requests;
	vector<MPI_Request> recv_requests;
	vector<CoordsValue*> recv_data;

	for (int i = 1; i < width + 1; i++) {
		for (int o = 1; o < height + 1; o++) {
			Coords c{i, o};
			bool send = canvas.at(c.x, c.y);
			Coords g = c.to_global();
			int tag = (int) c.to_global_tag();
			// Sides
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
//	cout << "Expected send count" << (width + height) * 2 + 4 << endl;
//	cout << "Expected recv count" << (width + height) * 2 + 4 << endl;
	cout << "Send count" << send_requests.size() << endl;
	cout << "Recv count" << recv_requests.size() << endl;
	cout << "T" << world_rank << " Waiting for all" << endl;
//	MPI_Waitall((int) recv_requests.size(), recv_requests.data(), MPI_STATUSES_IGNORE);
	MPI_Manual_Waitall((int) recv_requests.size(), recv_requests.data(), MPI_STATUSES_IGNORE);

	cout << "T" << world_rank << " Donne" << endl;
	for (int i = 0; i < recv_requests.size(); i++) {
		cout << "Rank: " << world_rank << " got " << recv_data[i]->pos << " " << recv_data[i]->value << endl;
		canvas.at(recv_data[i]->pos.x, recv_data[i]->pos.y) = recv_data[i]->value;
	}
	MPI_Manual_Waitall((int) send_requests.size(), send_requests.data(), MPI_STATUSES_IGNORE);
	// TODO: Free recv_data items
}


void recv_request(vector<MPI_Request> &recv_requests, vector<CoordsValue *> &recv_data) {
	auto *cv = new CoordsValue;
	MPI_Request req;
	int irecv = MPI_Irecv(cv, 1, MPI_CoordsValue, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &req);
	if (irecv != MPI_SUCCESS) {
		char error_string[MPI_MAX_ERROR_STRING];
		int length_of_error_string;
		MPI_Error_string(irecv, error_string, &length_of_error_string);
		fprintf(stderr, "MPI_Irecv error: %s\n", error_string);
		// Handle the error appropriately
	}
	recv_requests.push_back(req);
	recv_data.push_back(cv);
}

void send_request(vector<MPI_Request> &send_requests, int dest, CoordsValue cv) {
	MPI_Request req;
	int ierr = MPI_Isend(&cv, 1, MPI_CoordsValue, dest, 0, MPI_COMM_WORLD, &req);
	if (ierr != MPI_SUCCESS) {
		char error_string[MPI_MAX_ERROR_STRING];
		int length_of_error_string;
		MPI_Error_string(ierr, error_string, &length_of_error_string);
		fprintf(stderr, "MPI_Isend error: %s\n", error_string);
		// Handle the error appropriately
	}
	send_requests.push_back(req);
}


int main() {
	MPI_Init(nullptr, nullptr);
	init_datatypes();
	create_topo();
	init_relative();
//    debug_ranks();
//	if (world_rank == ANALYSIS_RANK)
//		debug_neighbors();
	canvas.init();
//	if (world_rank != ANALYSIS_RANK) {
//    for (int i = 0; i < 10; i++) {
//        for (int o = 0; o < 10; o++) {
//            const Coords &glob = Coords{i, o}.to_global();
//            canvas.at(i, o) = glob.x == 8 && glob.y == 7;
//        }
//    }
	if (world_rank == ANALYSIS_RANK) {
		canvas.at(1, 1) = true;
	}
//	}
//    print_all3();
////	if (world_rank == ANALYSIS_RANK) {
//////		canvas.load_file("data/mini.rle");
////        canvas.at(1, 5) = true;
////	}
    debug_neighbors();
	comunicate();
//    usleep(1000000);
//	print_all2();
//	usleep(1000000);
//	print_all3();
//	comunicate();
//    print_all2();
//    usleep(1000000);

//	for (int i = 0; i < 10; i++) {
//		comunicate();
//		print_all2();
//		canvas.iter();
//	}
	MPI_Barrier(MPI_COMM_WORLD);
//    usleep(1000000);
//	cout << '\n' << '\n';
//	print_all();
	MPI_Finalize();
	return 0;
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
			cout << "\033[" << (g.x * 2) + 1 << ";" << (g.y * 4) + 1 << "H" << "\033[3" << world_rank + 1 << "m"
			     //			     << (canvas.at(c.x, c.y) ? 'X' : '.') << hex << /*faint*/ "\033[2m" <<
			     << c.to_global_tag() << hex << /*faint*/ "\033[2m" <<
			     world_rank << dec << "\033[0m";
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	cout << "\033[0m\033[" << (height + 2) * dy + 1 << ";1H" << flush;
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

