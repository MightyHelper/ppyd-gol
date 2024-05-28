#include <cstdio>
#include "../include/utils.h"


const char *Utils::ansi_colors(unsigned int id){
	// Generate a color based on an ID, adjacent colors should be sufficiently different

	// Compute a hash of the ID
	unsigned int r = ((id + 13) * 123) % 256;
	unsigned int g = ((id + 442) * 321) % 256;
	unsigned int b = ((id + 4) * 213) % 256;
	// "\033[38;2;" r ";" g ";" b "m"
	char *buffer = new char[20];
	sprintf(buffer, "\033[38;2;%d;%d;%dm", r, g, b);
	return buffer;
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
