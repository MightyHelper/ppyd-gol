#include "utils.h"

template<typename T>
requires arithmetic<T>
T Utils<T>::clamp(T v, T m, T M) {
    return v < m ? m : (v > M ? M : v);
}

template<typename T>
requires arithmetic<T>
T Utils<T>::coords(T width, T x, T y) {
    return y * width + x;
}

template<typename T>
requires arithmetic<T>
T Utils<T>::positive_modulo(T a, T b) {
    return ((a + b) % b + b) % b;
}

template<typename T>
requires arithmetic<T>
T Utils<T>::clamped_coords(T width, T height, T x, T y) {
    return coords(width, clamp(x, 0, width - 1), clamp(y, 0, height - 1));
}

template<typename T>
requires arithmetic<T>
T Utils<T>::wrapped_coords(T width, T height, T x, T y) {
    return coords(width, positive_modulo(x, width), positive_modulo(y, height));
}

template
class Utils<int>;

template
class Utils<unsigned int>;
