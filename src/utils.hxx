#ifndef GOL_UTILS_HXX
#define GOL_UTILS_HXX

constexpr int clamp(int v, int m, int M) {
	return v < m ? m : (v > M ? M : v);
}

constexpr int coords(int width, int x, int y) {
	return y * width + x;
}

constexpr int positive_modulo(int a, int b) {
	return (a % b + b) % b;
}

constexpr int clamped_coords(int width, int height, int x, int y) {
	return coords(width, clamp(x, 0, width - 1), clamp(y, 0, height - 1));
}

constexpr int wrapped_coords(int width, int height, int x, int y) {
	return coords(width, positive_modulo(x, width), positive_modulo(y, height));
}

#endif //GOL_UTILS_HXX
