//
// Created by federico on 5/9/24.
//

#ifndef GOL_CANVAS_HXX
#define GOL_CANVAS_HXX

#include <bitset>
#include "utils.hxx"
#include <iostream>
#include <fstream>
#include <sstream>
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

    void rle_decode_line(const std::string &str, int row);

    int get_total(int x, int y);

    int step(int x, int y);

    void step_all();

public:
    Canvas();

    void generate(int &ib, int row, bool value, int count);
};

#define CLEAR_SCREEN "\033[2J"
#define GOTO_0_0 "\033[0;0H"

#define cchar(i) center[i*3] << center[i*3+1] << center[i*3+2]
#define bchar(i) box[i*3] << box[i*3+1] << box[i*3+2]
const char *center = "▒█";
const char *box = "┌─┐│█│└─┘";

#include "utils.hxx"

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
        for (int o = 0; o < width; o++) ss << get_total(i, o) << " ";
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
            (*cells)[clamped_coords<width, height>(i, o)] = 0;
            (*buffer)[clamped_coords<width, height>(i, o)] = 0;
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
void Canvas<width, height>::generate(int& ib, int row, bool value, int count){
    std::cout << "generate(" << ib << ", " << row << ", " << value << ", " << count << ")" << std::endl;
    for (int i = 0; i < count; i++) {
        std::cout << (value ? 'b' : 'o');
        at(ib++, row) = value;
    }
}

bool is_number(char c) {
    return c >= '0' && c <= '9';
}

std::string next_token(const std::string &str, int &offset) {
    // if number, read until non-number
    // if letter just return letter
    std::string out;
    if (offset >= str.length()) return out;
    if (is_number(str[offset])) {
        while (is_number(str[offset])) {
            out += str[offset++];
        }
    } else {
        out = str[offset++];
    }
    return out;
}

template<int width, int height>
void Canvas<width, height>::rle_decode_line(const std::string &str, int row) {
    std::string number;
    std::cout << row << "[" << str << "] " << ": ";
    int ib = 0;
    int offset = 0;
    std::string next = next_token(str, offset);
    while (offset < str.length()) {
        if (is_number(next[0])) {
            int n = std::stoi(next);
            std::cout << "Number: " << n << std::endl;
            next = next_token(str, offset);
            generate(ib, row, next == "b", n);
            next = next_token(str, offset);
        } else {
            std::cout << "Letter: " << next << std::endl;
            generate(ib, row, next == "b", 1);
            next = next_token(str, offset);
        }
    }
    std::cout << std::endl;
}

template<int width, int height>
void Canvas<width, height>::load_file(const char *path) {
    std::ifstream f(path);
    std::stringstream buf;
    if (!f) throw std::runtime_error(std::string("File ") + path + " not found.");
    buf << f.rdbuf();
    std::string data = buf.str();
    init();
    // - Split by newlines
    // - Filter out comments
    // - Drop the first line
    // - Transform the range into a string_view
    // - Join the string_views
    // - Split by '$'
    while (data[0] == '#') {
        data = data.substr(data.find('\n') + 1);
    }
    data = data.substr(data.find('\n') + 1);
    // Remove '\n'
    data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());
    int row = 0;
    while (data[0] == '$') {
        unsigned long idx = data.find('$', 1);
        if (idx == std::string::npos) break;
        std::cout << data << " " << idx<< std::endl;
        const std::string str = data.substr(1, idx - 1);
        rle_decode_line(str, row++);
        print();
        data = data.substr(idx);
    }
    const std::string str = data.substr(1, data.length() - 1);
    rle_decode_line(str, row);
    print();
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


#endif //GOL_CANVAS_HXX
