//
// Created by federico on 5/8/24.
//

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include "golio.h"
#include "utils.h"

#define cchar(i) center[i*3] << center[i*3+1] << center[i*3+2]
#define bchar(i) box[i*3] << box[i*3+1] << box[i*3+2]
const char *center = "▒█";
const char *box = "┌─┐│█│└─┘";

template<int width, int height>
State<width, height>::State(){

}
template<int width, int height>
std::bitset<width * height>::reference State<width, height>::at(int x, int y) const {
    return cells[clamped_coords(x, y)];
}

template<int width, int height>
void State<width, height>::print() const {
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
void State<width, height>::raw_print() const {
    std::stringstream ss;
    for (int i = 0; i < height; i++) {
        for (int o = 0; o < width; o++) ss << at(i, o) << " ";
        ss << "\n";
    }
    std::cout << ss.str() << std::flush;
}

template<int width, int height>
void State<width, height>::raw_print_idx() const {
    std::stringstream ss;
    for (int i = 0; i < height; i++) {
        for (int o = 0; o < width; o++) ss << clamped_coords(i, o) << " ";
        ss << "\n";
    }
    std::cout << ss.str() << std::flush;
}

template<int width, int height>
void State<width, height>::raw_print_total() const {
    std::stringstream ss;
    for (int i = 0; i < height; i++) {
        for (int o = 0; o < width; o++)ss << get_total(i, o) << " ";
        ss << "\n";
    }
    std::cout << ss.str() << std::flush;
}

template<int width, int height>
void State<width, height>::init() {
    cells = new std::bitset<width * height>;
    buffer = new std::bitset<width * height>;
    for (int i = 0; i < height; i++) {
        for (int o = 0; o < width; o++) {
            cells[clamped_coords(i, o)] = 0;
            buffer[clamped_coords(i, o)] = 0;
        }
    }
}


template<int width, int height>
void State<width, height>::spawn(std::string data, int ox, int oy) {
    int x = ox;
    int y = oy;
    for (int i = 0; i < data.length(); i++) {
        switch (data[i]) {
            case '\n':
                x = ox;
                y++;
                break;
            case '.':
                cells[clamped_coords(x, y)] = 0;
                x++;
                break;
            case 'X':
                cells[clamped_coords(x, y)] = 1;
                x++;
        }
    }
}

template<int width, int height>
void State<width, height>::iter() {
    step_all();
    void *temp = cells;
    cells = buffer;
    buffer = (int *) temp;
}


void rle_decode_line(std::string str, std::ranges::view){

}

template<int width, int height>
void State<width, height>::load_file(const char *path) {
    std::ifstream f(path);
    std::stringstream buf;
    if (!f) throw std::runtime_error(std::string("File ") + path + " not found.");
    buf << f.rdbuf();
    std::string data = buf.str();
    // Using std::ranges:
    // - Filter out lines that start with #
    // - Join all lines into a single string, without delimiter, effectively removing newlines
    // - Split on '$' character
    // - enumerate each item
    // - for each item, run through rle_decode_line(item, X); where X is a view on 'buffer' starting at width*item_index
    auto out = data
        | std::ranges::views::split('\n')
        | std::ranges::views::filter([](auto &line) { return line[0] != '#'; })
        | std::ranges::views::join
//        | std::ranges::views::split('$')
//        | std::ranges::views::enumerate
//        | std::ranges::views::for_each([this](auto &item) {
//            rle_decode_line(item, std::views::subrange(buffer, width * item.index, width * (item.index + 1)));
//        });
    cout << out;

}

template<int width, int height>
std::string &State<width, height>::advance_block(int x, int y, char last_type, std::string &count) {
    int i;
    if (!count.empty()) {
        i = atoi(count.c_str());
    } else {
        i = 1;
    }
    std::cout << i << "*" << last_type << std::endl;
    int cnt = x + i;
    for (; x < cnt; x++) {
        cells[clamped_coords(x, y)] = last_type == 'b';
    }
    count = "";
    return count;
}


template<int width, int height>
int State<width, height>::get_total(int x, int y) {
    int total = 0;
    for (int i = -1; i < 2; i++)
        for (int o = -1; o < 2; o++)
            total += at(x + i, y + o);
    return total - at(x, y);
}

template<int width, int height>
int State<width, height>::step(int x, int y) {
    int total = get_total<width, height>(x, y);
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
void State<width, height>::step_all() {
    for (int i = 0; i < height; i++) for (int o = 0; o < width; o++) at(i, o) = step(i, o);
}

