#include "canvas.h"
#include "vec2.h"
#include "mpi_utils.h"
#include <atomic>
#include <array>
#include <sstream>
#include <functional>
#include <memory>

using namespace std;

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

class MPICanvas {
public:
    unique_ptr<Canvas> canvas;
    unsigned int rank;
    unsigned int size;
    array<unsigned int, 9> relative{};
    MPI_Comm comm;
    Vec2<unsigned int> item_size;
    Vec2<unsigned int> cart_coords{};
    Vec2<unsigned int> dims{};
    Vec2<unsigned int> periods;

    MPICanvas(
            Canvas *canvas,
            Vec2<unsigned int> item_size,
            Vec2<unsigned int> periods = Vec2<unsigned int>{0, 0}
    ) : canvas(canvas), comm(), item_size(item_size), periods(periods) {
        this->rank = get_rank();
        this->size = get_size();
        init_topology();
        this->relative = init_relative();
    }

    void comunicate() {
        // 0 1 2 <- [ Old scheme ]    0 3 6 | [ Conversion table ]
        // 3 4 5    [ New scheme ] -> 1 4 7 | [0, 4, 8] the same
        // 6 7 8                      2 5 8 | [1 <=> 3] [2 <=> 6] [5 <=> 7]
        vector<MPI_Request> send_requests{};
        vector<MPI_Request> recv_requests{};
        vector<CoordsValue *> send_data{};
        vector<CoordsValue *> recv_data{};

        send_requests.reserve((item_size.x + item_size.y + 2) * 2);
        recv_requests.reserve((item_size.x + item_size.y + 2) * 2);
        send_data.reserve((item_size.x + item_size.y + 2) * 2);
        recv_data.reserve((item_size.x + item_size.y + 2) * 2);

        for (unsigned int i = 1; i < item_size.x + 1; i++) {
            for (unsigned int o = 1; o < item_size.y + 1; o++) {
                Vec2 c{i, o};
                bool send = canvas->at(c.x, c.y);
                Vec2 g = c.to_global(cart_coords, item_size, dims);
                if (c.x == 1) send_request(send_requests, send_data, relative[3], new CoordsValue{g, send});
                if (c.x == item_size.x) send_request(send_requests, send_data, relative[5], new CoordsValue{g, send});
                if (c.y == 1) send_request(send_requests, send_data, relative[1], new CoordsValue{g, send});
                if (c.y == item_size.y) send_request(send_requests, send_data, relative[7], new CoordsValue{g, send});
                if (c.x == 1 && c.y == 1) send_request(send_requests, send_data, relative[0], new CoordsValue{g, send});
                if (c.x == 1 && c.y == item_size.y)
                    send_request(send_requests, send_data, relative[6], new CoordsValue{g, send});
                if (c.x == item_size.x && c.y == 1)
                    send_request(send_requests, send_data, relative[2], new CoordsValue{g, send});
                if (c.x == item_size.x && c.y == item_size.y)
                    send_request(send_requests, send_data, relative[8], new CoordsValue{g, send});
            }
        }
        for (unsigned int i = 0; i < item_size.x + 2; i++) {
            for (unsigned int o = 0; o < item_size.y + 2; o++) {
                Vec2 c{i, o};
                if (c.x != 0 && c.y != 0 && c.x != item_size.x + 1 && c.y != item_size.y + 1) continue;
                recv_request(recv_requests, recv_data);
            }
        }
        MPI_Waitall((int) recv_requests.size(), recv_requests.data(), MPI_STATUSES_IGNORE);
        for (unsigned int i = 0; i < recv_requests.size(); i++) {
            Vec2 local = recv_data[i]->pos.global_to_local(cart_coords, item_size, dims);
            canvas->at(local.x, local.y) = recv_data[i]->value;
        }
        MPI_Waitall((int) send_requests.size(), send_requests.data(), MPI_STATUSES_IGNORE);
//        for (auto &i: recv_data) delete i;
//        for (auto &i: recv_requests) delete i;
    }

//private:
    static int get_rank() {
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        return rank;
    }

    static int get_size() {
        int size;
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        return size;
    }

    void init_topology() {
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

    [[nodiscard]] array<unsigned int, 9> init_relative() const {
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

    static void recv_request(vector<MPI_Request> &recv_requests, vector<CoordsValue *> &recv_data) {
        auto *c = new CoordsValue{};
        MPI_Request req = 0;
        int irecv = MPI_Irecv(c, 1, MPIUtils::MPI_CoordsValue, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &req);
        validateMPIOutput(irecv);
        recv_requests.push_back(req);
        recv_data.push_back(c);
    }

    static void
    send_request(vector<MPI_Request> &send_requests, vector<CoordsValue *> &send_data, unsigned int dest,
                 CoordsValue *cv) {
        MPI_Request req = 0;
        int ierr = MPI_Isend(cv, 1, MPIUtils::MPI_CoordsValue, (int) dest, 0, MPI_COMM_WORLD, &req);
        validateMPIOutput(ierr);
        send_requests.push_back(req);
        send_data.push_back(cv);
    }


};
namespace Debug {

    void print_all(const MPICanvas &canvas) {
        for (unsigned int i = 0; i < 10; i++) {
            MPI_Barrier(MPI_COMM_WORLD);
            if (canvas.rank == i) {
                cout << canvas.rank << endl;
                canvas.canvas->print();
            }
            usleep(10000);
        }
    }


    void print_all_generic(
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

    void print_all_full_generic(
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
    void print_all_full_generic_line(
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

    void print_all5(const MPICanvas &canvas, Vec2<unsigned int> base = {0, 0}) {
        print_all_generic(
                canvas,
                base,
                Vec2<unsigned int>{2, 1},
                [&canvas](stringstream &f, Vec2<unsigned int> c, Vec2<unsigned int> g) -> void {
                    f << ansi_colors[canvas.rank] << (canvas.canvas->at(c.x, c.y) ? "██" : "░░");
                }
        );
    }

    void print_all5_full(const MPICanvas &canvas, Vec2<unsigned int> base = {0, 0}) {
        print_all_full_generic(
                canvas,
                base,
                Vec2<unsigned int>{2, 1},
                [&canvas](stringstream &f, Vec2<unsigned int> c, Vec2<unsigned int> g) -> void {
                    f << ansi_colors[canvas.rank] << (canvas.canvas->at(c.x, c.y) ? "██" : "░░");
                }
        );
    }

    void print_all_full_all(const MPICanvas &canvas, Vec2<unsigned int> base = {0, 0}) {
        Debug::print_all_full_generic(
                canvas,
                base,
                Vec2<unsigned int>{12, 2},
                [&canvas](stringstream &f, Vec2<unsigned int> c, Vec2<unsigned int> g) {
                    Vec2<unsigned int> g2 = c.to_global(canvas.cart_coords, canvas.item_size, canvas.dims);
                    const char* und = canvas.canvas->at(c.x, c.y) ? "\033[4m" : "";
                    f << und << ansi_colors[canvas.rank]
                      << c.x << "|" << c.y << /*dim*/ "\033[2m"
                      << g.x << "|" << g.y << "\033[0m" <<und <<  ansi_colors[canvas.rank] << "\033[1m"
                      << g2.x << "|" << g2.y;
                }
        );
    }

    void debug_ranks(const MPICanvas &canvas) {
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

    void debug_neighbors(const MPICanvas &canvas) {
        stringstream x;
        x << "Rank: " << canvas.rank << " Vec2: " << canvas.cart_coords.x << " " << canvas.cart_coords.y << '\n';
        for (int i = 0; i < 9; i++) {
            x << canvas.relative[i] << " ";
            if ((i + 1) % 3 == 0) x << '\n';
        }
        x << '\n';
        cout << x.str();
    }

    void plot_topo(const MPICanvas &canvas) {
        if (canvas.rank == 0) cout << "\033[2J" << "\033[0;0H" << flush;
        MPI_Barrier(MPI_COMM_WORLD);
        cout << "\033[" << canvas.cart_coords.x + 1 << ";" << canvas.cart_coords.y + 1 << "H" << "x";
        MPI_Barrier(MPI_COMM_WORLD);
        cout << "\n\n" << flush;
    }

    void debug_mpi_datatypes(const MPICanvas &canvas) {
        const unsigned int root = 0;
        vector<MPI_Request> requests{};
        vector<CoordsValue *> sends{};
        if (canvas.rank == root) {
            for (unsigned int i = 0; i < canvas.size; i++) {
                MPICanvas::send_request(requests, sends, i,
                                        new CoordsValue{Vec2<unsigned int>{root + 69, root + 42}, true});
            }
        }
        vector<MPI_Request> responses{};
        vector<CoordsValue *> recvs{};
        MPICanvas::recv_request(responses, recvs);
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

}
