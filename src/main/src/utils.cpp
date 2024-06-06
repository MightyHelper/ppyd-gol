#include <cstdio>
#include "../include/utils.h"

/*
 * Utils::ansi_colors function: This function generates an ANSI color code based on an ID.
 * It computes a hash of the ID to generate RGB color values, and then formats these values into an ANSI
 * color code string. The function ensures that adjacent colors should be sufficiently different.
 * */
const char *Utils::ansi_colors(unsigned int id) {
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

/*
 * MathUtils::clamp function: This function clamps a value v between a minimum m and a maximum M. If v
 * is less than m, it returns m. If v is greater than M, it returns M. Otherwise, it returns v.
 * */
template<typename T>
requires arithmetic<T>
T MathUtils<T>::clamp(T v, T m, T M) {
	return v < m ? m : (v > M ? M : v);
}

/*
 * MathUtils::coords function: This function calculates a linear index from 2D coordinates x and y given
 * a width. This is typically used for accessing elements in a 1D array that represents a 2D grid.
 * */
template<typename T>
requires arithmetic<T>
T MathUtils<T>::coords(T width, T x, T y) {
	return y * width + x;
}

/*
 * MathUtils::positive_modulo function: This function calculates the positive modulo of a and b. This is
 * used to wrap values around in a range from 0 to b.
 * */
template<typename T>
requires arithmetic<T>
T MathUtils<T>::positive_modulo(T a, T b) {
	return ((a + b) % b + b) % b;
}

/*
 * MathUtils::clamped_coords function: This function calculates a linear index from 2D coordinates x and
 * y given a width and height, clamping the coordinates to the range [0, width-1] and [0, height-1]
 * respectively.
 * */
template<typename T>
requires arithmetic<T>
T MathUtils<T>::clamped_coords(T width, T height, T x, T y) {
	return coords(width, clamp(x, 0, width - 1), clamp(y, 0, height - 1));
}

/*
 * MathUtils::wrapped_coords function: This function calculates a linear index from 2D coordinates x and
 * y given a width and height, wrapping the coordinates around in the range [0, width] and [0, height]
 * respectively.
 */
template<typename T>
requires arithmetic<T>
T MathUtils<T>::wrapped_coords(T width, T height, T x, T y) {
	return coords(width, positive_modulo(x, width), positive_modulo(y, height));
}

/*
 * The MathUtils class is a template class that requires the template parameter T to be an arithmetic
 * type. This means that the functions can work with any type that supports arithmetic operations,
 * such as int, float, double, etc. The class is explicitly instantiated for int and unsigned int types
 * at the end of the file.
 * */
template
class MathUtils<int>;

template
class MathUtils<unsigned int>;
