#ifndef GOL_UTILS_H
#define GOL_UTILS_H

#include <concepts>

template<typename T>
concept arithmetic = std::integral<T> || std::floating_point<T>;

#define rgb(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m"
// List of rgb ansi colors

namespace Utils{
  extern const char *ansi_colors[];
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
