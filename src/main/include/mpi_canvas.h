#ifndef GOL_MPI_CANVAS_H
#define GOL_MPI_CANVAS_H

#include "vec2.h"
#include "canvas.h"
#include <mpi.h>
#include <array>
#include <memory>

class MPICanvas {
public:
    std::unique_ptr<Canvas> canvas;
    unsigned int rank;
    unsigned int size;
    std::array<unsigned int, 9> relative{};
    MPI_Comm comm;
    Vec2<unsigned int> item_size;
    Vec2<unsigned int> cart_coords{};
    Vec2<unsigned int> dims{};
    Vec2<unsigned int> periods;

    MPICanvas(
            Canvas *canvas,
            Vec2<unsigned int> item_size,
            Vec2<unsigned int> periods = Vec2<unsigned int>{0, 0}
    );

    void comunicate();

		void load_file(const std::string &str, int dx = 0, int dy = 0);

    void iter();

private:
    void init_topology();

    [[nodiscard]] std::array<unsigned int, 9> init_relative() const;
};


#endif //GOL_MPI_CANVAS_H
