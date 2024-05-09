//
// Created by federico on 5/9/24.
//

#ifndef GOL_CANVAS_H
#define GOL_CANVAS_H

#include <bitset>
#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ranges>
#include <vector>

template<int width, int height>
class Canvas {
public:
    std::bitset<width * height> *cells;
    std::bitset<width * height> *buffer;

    std::bitset<width * height>::reference at(int x, int y) const;

    void print() const;

    void raw_print() const;

    void raw_print_idx() const;

    void raw_print_total() const;

    void init();

    void spawn(std::string data, int ox, int oy);

    void iter();

    void load_file(const char *path);

    void rle_decode_line(std::string &str, int offset);

    std::string &advance_block(int x, int y, char last_type, std::string &count);

    int get_total(int x, int y);

    int step(int x, int y);

    void step_all();

public:
    Canvas();
};
//
// Created by federico on 5/9/24.
//

#define CLEAR_SCREEN "\033[2J"
#define GOTO_0_0 "\033[0;0H"

#define cchar(i) center[i*3] << center[i*3+1] << center[i*3+2]
#define bchar(i) box[i*3] << box[i*3+1] << box[i*3+2]
const char *center = "▒█";
const char *box = "┌─┐│█│└─┘";

#include "utils.h"

template<int width, int height>
Canvas<width, height>::Canvas() = default;

template<int width, int height>
std::bitset<width * height>::reference Canvas<width, height>::at(int x, int y) const {
    return (*cells)[clamped_coords<width, height>(x, y)];
}

template<int width, int height>
void Canvas<width, height>::print() const {
    std::stringstream ss;
    ss << bchar(0);
    for (int o = 0; o < width; o++) ss << bchar(1) << bchar(1);
    ss << bchar(2);
    ss << "\n";
    for (int i = 0; i < height; i++) {
        ss << bchar(3);
        for (int o = 0; o < width; o++) ss << cchar(at(i, o)) << cchar(at(i, o));
        ss << bchar(5) << "\n";
    }
    ss << bchar(6);
    for (int o = 0; o < width; o++)ss << bchar(7) << bchar(7);

    ss << bchar(8) << "\n";
    std::cout << ss.str() << std::flush;
}

template<int width, int height>
void Canvas<width, height>::raw_print() const {
    std::stringstream ss;
    for (int i = 0; i < height; i++) {
        for (int o = 0; o < width; o++) ss << at(i, o) << " ";
        ss << "\n";
    }
    std::cout << ss.str() << std::flush;
}

template<int width, int height>
void Canvas<width, height>::raw_print_idx() const {
    std::stringstream ss;
    for (int i = 0; i < height; i++) {
        for (int o = 0; o < width; o++) ss << clamped_coords<width, height>(i, o) << " ";
        ss << "\n";
    }
    std::cout << ss.str() << std::flush;
}

template<int width, int height>
void Canvas<width, height>::raw_print_total() const {
    std::stringstream ss;
    for (int i = 0; i < height; i++) {
        for (int o = 0; o < width; o++)ss << get_total(i, o) << " ";
        ss << "\n";
    }
    std::cout << ss.str() << std::flush;
}

template<int width, int height>
void Canvas<width, height>::init() {
    cells = new std::bitset<width * height>;
    buffer = new std::bitset<width * height>;
    for (int i = 0; i < height; i++) {
        for (int o = 0; o < width; o++) {
            cells[clamped_coords<width, height>(i, o)] = 0;
            buffer[clamped_coords<width, height>(i, o)] = 0;
        }
    }
}


template<int width, int height>
void Canvas<width, height>::spawn(std::string data, int ox, int oy) {
    int x = ox;
    int y = oy;
    for (int i = 0; i < data.length(); i++) {
        switch (data[i]) {
            case '\n':
                x = ox;
                y++;
                break;
            case '.':
                cells[clamped_coords<width, height>(x, y)] = 0;
                x++;
                break;
            case 'X':
                cells[clamped_coords<width, height>(x, y)] = 1;
                x++;
        }
    }
}

template<int width, int height>
void Canvas<width, height>::iter() {
    step_all();
    void *temp = cells;
    cells = buffer;
    buffer = (int *) temp;
}


template<int width, int height>
void Canvas<width, height>::rle_decode_line(std::string &str, int offset) {
    std::string number;
    for (char c: str) {
        if (c >= '0' && c <= '9') {
            number += c;
        } else {
            for (int i = 0; i < atoi(number.c_str()); i++, offset++) {
                cells[offset] = c == 'b';
            }
        }
    }
}

namespace r = std::ranges;
namespace v = r::views;

std::vector<std::string> view_to_vec(auto view){
    std::vector<std::string> out_vec;
    for (auto strings: view) {
        for (auto string: strings) {
            std::stringstream a;
            a << string;

            out_vec.push_back(std::string(a.str()));
        }
    }
    return out_vec;
}

template<int width, int height>
void Canvas<width, height>::load_file(const char *path) {
    std::ifstream f(path);
    std::stringstream buf;
    if (!f) throw std::runtime_error(std::string("File ") + path + " not found.");
    buf << f.rdbuf();
    std::string data = buf.str();
    // Using std::ranges:
    // - Filter out lines that start with #
    // - Join all lines into a single string, without delimiter, effectively removing newlines
    // - Split on '$' character
    // - materialize to a vector of strings
    auto out = data
               | v::split('\n')
               | v::filter([](auto &&line) { return *line.begin() != '#'; })
               | v::drop(1)
               | v::transform([](auto &&rng) { return std::string_view(&*rng.begin(), r::distance(rng)); })
               | v::join
               | v::split('$');
    std::vector<std::string> out_vec = view_to_vec(out);
//    std::cout << std::string(out) << std::endl;
    for (auto string: out_vec) {
        std::cout << string << std::endl;
    }

}

template<int width, int height>
std::string &Canvas<width, height>::advance_block(int x, int y, char last_type, std::string &count) {
    int i;
    if (!count.empty()) {
        i = atoi(count.c_str());
    } else {
        i = 1;
    }
    std::cout << i << "*" << last_type << std::endl;
    int cnt = x + i;
    for (; x < cnt; x++) {
        cells[clamped_coords<width, height>(x, y)] = last_type == 'b';
    }
    count = "";
    return count;
}


template<int width, int height>
int Canvas<width, height>::get_total(int x, int y) {
    int total = 0;
    for (int i = -1; i < 2; i++)
        for (int o = -1; o < 2; o++)
            total += at(x + i, y + o);
    return total - at(x, y);
}

template<int width, int height>
int Canvas<width, height>::step(int x, int y) {
    int total = get_total < width, height>(x, y);
    int current = at(x, y);
    if (current) {
        if (total < 2) return 0;
        if (total < 4) return 1;
        if (total == 4) return 0;
    }
    if (total == 3) return 1;
    return 0;
}

template<int width, int height>
void Canvas<width, height>::step_all() {
    for (int i = 0; i < height; i++) for (int o = 0; o < width; o++) at(i, o) = step(i, o);
}


#endif //GOL_CANVAS_H
