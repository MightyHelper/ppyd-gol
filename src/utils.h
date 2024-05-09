#ifndef GOL_UTILS_H
#define GOL_UTILS_H

constexpr int clamp(int v, int m, int M) {
    return v < m ? m : (v > M ? M : v);
}

template<int width>
constexpr int coords(int x, int y) {
    return x * width + y;
}
template<int width, int height>
constexpr int clamped_coords(int x, int y) {
    return coords<width>(clamp(x, 0, width - 1), clamp(y, 0, height - 1));
}

#endif //GOL_UTILS_H
