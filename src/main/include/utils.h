#ifndef GOL_UTILS_H
#define GOL_UTILS_H

#include <concepts>

template<typename T>
concept arithmetic = std::integral<T> || std::floating_point<T>;

namespace Utils{
  const char *ansi_colors(unsigned int index);
};

template<typename T> requires arithmetic<T>
class MathUtils {
public:
  static T clamp(T v, T m, T M);
  static T coords(T width, T x, T y);
  static T positive_modulo(T a, T b);
  static T clamped_coords(T width, T height, T x, T y);
  static T wrapped_coords(T width, T height, T x, T y);
};


#endif //GOL_UTILS_H
