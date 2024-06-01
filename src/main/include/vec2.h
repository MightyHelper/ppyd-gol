//
// Created by federico on 5/26/24.
//

#ifndef GOL_VEC2_H
#define GOL_VEC2_H

#include <iostream>
#include <map>
#include <unordered_map>

// Define the static member outside the class template definition
template<typename T> requires std::integral<T>
std::unordered_map<T, T> global_to_local_cache_a;
template<typename T> requires std::integral<T>
std::unordered_map<T, T> global_to_local_cache_b;

template<typename T> requires std::integral<T>
class Vec2 {
public:
  T x;
  T y;
  [[nodiscard]] Vec2 global_to_local(Vec2 cart_coords, Vec2 size, Vec2 dims) const;
  [[nodiscard]] Vec2 global_to_local_memoized(Vec2 cart_coords, Vec2 size, Vec2 dims) const;
  [[nodiscard]] Vec2 to_global(Vec2 cart_coords, Vec2 size, Vec2 dims) const;
  [[nodiscard]] Vec2 to_super_global(Vec2 cart_coords, Vec2 size) const;
  [[nodiscard]] T to_global_tag(Vec2 cart_coords, Vec2 size, Vec2 dims) const;
  [[nodiscard]] T global_to_tag(Vec2 cart_coords, Vec2 size, Vec2 dims) const;
  [[nodiscard]] T to_super_global_tag(Vec2 cart_coords, Vec2 size, Vec2 dims) const;

  bool operator==(const Vec2 &rhs) const;

  [[nodiscard]] T *array() const;

  size_t operator()(const Vec2<T> &p) const {
    auto hash1 = std::hash<T>{}(p.x);
    auto hash2 = std::hash<T>{}(p.y);

    if (hash1 != hash2) {
      return hash1 ^ hash2;
    }

    // If hash1 == hash2, their XOR is zero.
    return hash1;
  }

  template<typename V>
  requires std::integral<V>
  friend std::ostream &operator<<(std::ostream &os, const Vec2<V> &c);

  static void pre_build(Vec2<T> cart_coords, Vec2<T> size, Vec2<T> dims) {
    for (T i = 0; i < size.x + 2; i++) {
      for (T o = 0; o < size.y + 2; o++) {
        if (i == 1 || o == 1 || i == size.x || o == size.y || i == 0 || o == 0 || i == size.x + 1 || o == size.y + 1) {
          const Vec2 &vec2 = Vec2{i, o};
          Vec2 v = vec2.to_global(cart_coords, size, dims);
          T tag = v.global_to_tag(cart_coords, size, dims);
          Vec2 result = v.global_to_local(cart_coords, size, dims); // Implement this function
          global_to_local_cache_a<T>[tag] = result.x;
          global_to_local_cache_b<T>[tag] = result.y;
        }
      }
    }
  }
};


struct CoordsValue {
  Vec2<unsigned int> pos;
  bool value;
};

#endif //GOL_VEC2_H
