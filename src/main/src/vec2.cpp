//
// Created by federico on 5/26/24.
//

#include "../include/vec2.h"
#include "../include/utils.h"

using namespace std;

template<typename T>
requires std::integral<T>
[[nodiscard]] Vec2<T> Vec2<T>::global_to_local(Vec2<T> cart_coords, Vec2<T> size, Vec2<T> dims) const {
  // Todo: Optimize this
  for (T i = 0; i < size.x + 2; i++) {
    for (T o = 0; o < size.y + 2; o++) {
      if (i == 0 || o == 0 || i == size.x + 1 || o == size.y + 1 || i == 1 || o == 1 || i == size.x|| o == size.y) {
        const Vec2<T> c = Vec2<T>{i, o};
        const Vec2<T> g = c.to_global(cart_coords, size, dims);
        if (g.x == x && g.y == y)return c;
      }
    }
  }
  char buffer[100];
  sprintf(buffer, "Vec2<T>{%d,%d} not found in global_to_local", x, y);
  throw std::runtime_error(buffer);
}


template<typename T>
requires std::integral<T>
[[nodiscard]] Vec2<T> Vec2<T>::global_to_local_memoized(Vec2<T> cart_coords, Vec2<T> size, Vec2<T> dims) const {
  T tag = global_to_tag(cart_coords, size, dims);
  auto it = global_to_local_cache_a<T>.find(tag);
  if (it != global_to_local_cache_a<T>.end()) {
    auto it2 = global_to_local_cache_b<T>.find(tag);
    return Vec2{it->second, it2->second};
  }
//  cout << "MISS " << x << " " << y << " " << tag << endl;
  Vec2 result = global_to_local(cart_coords, size, dims); // Implement this function

  global_to_local_cache_a<T>[tag] = result.x;
  global_to_local_cache_b<T>[tag] = result.y;
  return result;
}


template<typename T>
requires std::integral<T>
[[nodiscard]] Vec2<T> Vec2<T>::to_global(Vec2<T> cart_coords, Vec2<T> size, Vec2<T> dims) const {
  return Vec2<T>{
   MathUtils<T>::positive_modulo(x + cart_coords.x * size.x + size.x * dims.x - 1, size.x * dims.x),
   MathUtils<T>::positive_modulo(y + cart_coords.y * size.y + size.y * dims.y - 1, size.y * dims.y)
  };
}

template<typename T>
requires std::integral<T>
[[nodiscard]] Vec2<T> Vec2<T>::to_super_global(Vec2<T> cart_coords, Vec2<T> size) const {
  return Vec2<T>{x + cart_coords.x * (size.x + 2), y + cart_coords.y * (size.y + 2)};
}

template<typename T>
requires std::integral<T>
[[nodiscard]] T Vec2<T>::to_global_tag(Vec2<T> cart_coords, Vec2<T> size, Vec2<T> dims) const {
  Vec2<T> g = to_global(cart_coords, size, dims);
  return MathUtils<T>::wrapped_coords(size.x * dims.x, size.y * dims.y, g.x, g.y);
}

template<typename T>
requires std::integral<T>
[[nodiscard]] T Vec2<T>::global_to_tag(Vec2<T> cart_coords, Vec2<T> size, Vec2<T> dims) const {
  return MathUtils<T>::wrapped_coords(size.x * dims.x, size.y * dims.y, x, y);
}

template<typename T>
requires std::integral<T>
[[nodiscard]] T Vec2<T>::to_super_global_tag(Vec2<T> cart_coords, Vec2<T> size, Vec2<T> dims) const {
  Vec2<T> g = to_super_global(cart_coords, size);
  return MathUtils<T>::wrapped_coords((size.x + 2) * dims.x, (size.y + 2) * dims.y, g.x, g.y);
}

template<typename T>
requires std::integral<T>
[[nodiscard]] T *Vec2<T>::array() const {
  return (T *) (this);
}

template<typename T>
requires std::integral<T>
bool Vec2<T>::operator==(const Vec2 &rhs) const {
  return x == rhs.x && y == rhs.y;
}

template<typename T>
requires std::integral<T>
ostream &operator<<(ostream &os, const Vec2<T> &c) {
  return os << "Vec2<T>{" << c.x << "," << c.y << "}";
}

template
class Vec2<int>;

template
class Vec2<unsigned int>;

template ostream &operator<<(ostream &os, const Vec2<int> &c);
template ostream &operator<<(ostream &os, const Vec2<unsigned int> &c);