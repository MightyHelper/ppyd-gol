//
// Created by federico on 5/26/24.
//

#include "vec2.h"
#include "utils.h"

using namespace std;

template<typename T>
requires std::integral<T>
[[nodiscard]] Vec2<T> Vec2<T>::global_to_local(Vec2<T> cart_coords, Vec2<T> size, Vec2<T> dims) const {
    // Todo: Optimize this
    for (T i = 0; i < size.x + 2; i++) {
        for (T o = 0; o < size.y + 2; o++) {
            const Vec2<T> c = Vec2<T>{i, o};
            const Vec2<T> g = c.to_global(cart_coords, size, dims);
//            cout << *this << " " << c << " " << g << endl;
            if (g.x == x && g.y == y) {
//                cout << "God!" << endl;
                return c;
            }
        }
    }
    char buffer[100];
    sprintf(buffer, "Vec2<T>{%d,%d} not found in global_to_local", x, y);
    throw std::runtime_error(buffer);
}


template<typename T>
requires std::integral<T>
[[nodiscard]] Vec2<T> Vec2<T>::to_global(Vec2<T> cart_coords, Vec2<T> size, Vec2<T> dims) const {
//    cout << cart_coords << " " << size << " " << dims << endl;
    return Vec2<T>{
            Utils<T>::positive_modulo(x + cart_coords.x * size.x + size.x * dims.x - 1, size.x * dims.x),
            Utils<T>::positive_modulo(y + cart_coords.y * size.y + size.y * dims.y - 1, size.y * dims.y)
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
    return Utils<T>::wrapped_coords(size.x * dims.x, size.y * dims.y, g.x, g.y);
}

template<typename T>
requires std::integral<T>
[[nodiscard]] T Vec2<T>::to_super_global_tag(Vec2<T> cart_coords, Vec2<T> size, Vec2<T> dims) const {
    Vec2<T> g = to_super_global(cart_coords, size);
    return Utils<T>::wrapped_coords((size.x + 2) * dims.x, (size.y + 2) * dims.y, g.x, g.y);
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