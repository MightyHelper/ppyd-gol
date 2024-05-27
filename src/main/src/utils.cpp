#include "../include/utils.h"

const char *Utils::ansi_colors[] = {
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

template<typename T>
requires arithmetic<T>
T MathUtils<T>::clamp(T v, T m, T M) {
  return v < m ? m : (v > M ? M : v);
}

template<typename T>
requires arithmetic<T>
T MathUtils<T>::coords(T width, T x, T y) {
  return y * width + x;
}

template<typename T>
requires arithmetic<T>
T MathUtils<T>::positive_modulo(T a, T b) {
  return ((a + b) % b + b) % b;
}

template<typename T>
requires arithmetic<T>
T MathUtils<T>::clamped_coords(T width, T height, T x, T y) {
  return coords(width, clamp(x, 0, width - 1), clamp(y, 0, height - 1));
}

template<typename T>
requires arithmetic<T>
T MathUtils<T>::wrapped_coords(T width, T height, T x, T y) {
  return coords(width, positive_modulo(x, width), positive_modulo(y, height));
}

template
class MathUtils<int>;

template
class MathUtils<unsigned int>;
