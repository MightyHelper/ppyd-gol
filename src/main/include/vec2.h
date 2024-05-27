//
// Created by federico on 5/26/24.
//

#ifndef GOL_VEC2_H
#define GOL_VEC2_H

#include <iostream>


template<typename T>
requires std::integral<T>
class Vec2 {
public:
    T x;
    T y;

    [[nodiscard]] Vec2 global_to_local(Vec2 cart_coords, Vec2 size, Vec2 dims) const;

    [[nodiscard]] Vec2 to_global(Vec2 cart_coords, Vec2 size, Vec2 dims) const;

    [[nodiscard]] Vec2 to_super_global(Vec2 cart_coords, Vec2 size) const;

    [[nodiscard]] T to_global_tag(Vec2 cart_coords, Vec2 size, Vec2 dims) const;

    [[nodiscard]] T to_super_global_tag(Vec2 cart_coords, Vec2 size, Vec2 dims) const;

    bool operator==(const Vec2 &rhs) const;

    [[nodiscard]] T *array() const;

    template<typename V>
    requires std::integral<V>
    friend std::ostream &operator<<(std::ostream &os, const Vec2<V> &c);
};

struct CoordsValue {
    Vec2<unsigned int> pos;
    bool value;
};

#endif //GOL_VEC2_H
