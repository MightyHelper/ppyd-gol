#include "canvas.h"
#include "utils.h"
#include <sstream>
#include <iostream>
#include <fstream>

#define cchar(i) center[i*3] << center[i*3+1] << center[i*3+2]
#define bchar(i) box[i*3] << box[i*3+1] << box[i*3+2]
//const char *center = "▒█";
const char *center = "░█";
const char *box = "┌─┐│█│└─┘";

using namespace std;

[[nodiscard]] bitset<BIT_GROUP_SIZE>::reference
Canvas::index(vector<bitset<BIT_GROUP_SIZE>> *mat, unsigned int index) const {
    return (*mat)[index / BIT_GROUP_SIZE][index % BIT_GROUP_SIZE];
}

[[nodiscard]] bitset<BIT_GROUP_SIZE>::reference Canvas::at(unsigned int x, unsigned int y) const {
    return index(cells, Utils<unsigned int>::clamped_coords(width, height, x, y));
}

[[nodiscard]] int Canvas::at_or0(unsigned int x, unsigned int y) const {
    if (x >= width || y >= height) return 0;
    return index(cells, Utils<unsigned int>::clamped_coords(width, height, x, y));
}

[[nodiscard]] std::bitset<BIT_GROUP_SIZE>::reference Canvas::buf(unsigned int x, unsigned int y) const {
    return index(buffer, Utils<unsigned int>::clamped_coords(width, height, x, y));
}

void Canvas::print() const {
    std::stringstream ss;
    ss << bchar(0);
    for (unsigned int o = 0; o < width; o++) ss << bchar(1) << bchar(1);
    ss << bchar(2);
    ss << "\n";
    for (unsigned int i = 0; i < height; i++) {
        ss << bchar(3);
        for (unsigned int o = 0; o < width; o++) ss << cchar(at(o, i)) << cchar(at(o, i));
        ss << bchar(5) << "\n";
    }
    ss << bchar(6);
    for (unsigned int o = 0; o < width; o++)ss << bchar(7) << bchar(7);

    ss << bchar(8) << "\n";
    std::cout << ss.str() << std::flush;
}

void Canvas::init() {
    for (unsigned int i = 0; i < cells->size(); i++){
        (*cells)[i] = 0;
        (*buffer)[i] = 0;
    }
    for (unsigned int i = 0; i < height * width; i++) {
        index(cells, i) = false;
        index(buffer, i) = false;
    }
}

void Canvas::spawn(std::string &data, int ox, int oy) const {
    int x = ox;
    int y = oy;
    for (unsigned int i = 0; i < data.length(); i++) {
        switch (data[i]) {
            case '\n':
                x = ox;
                y++;
                break;
            case '.':
                at(x, y) = false;
                x++;
                break;
            case 'X':
                at(x, y) = true;
                x++;
        }
    }
}

void Canvas::load_file(const std::string &str, int dx, int dy) {
    load_file(str.c_str(), dx, dy);
}

void Canvas::load_file(const char *path, int dx, int dy) {
    std::ifstream f(path);
    std::stringstream buf;
    if (!f) throw std::runtime_error(std::string("File ") + path + " not found.");
    buf << f.rdbuf();
    std::string data = buf.str();
    init();
    while (data[0] == '#') data = data.substr(data.find('\n') + 1);
    data = data.substr(data.find('\n') + 1);
    data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());
    int row = dy;
    do {
        unsigned long idx = data.find('$', 1);
        if (idx == std::string::npos) break;
        const std::string str = data.substr(1, idx - 1);
        rle_decode_line(str, row++, dx);
        data = data.substr(idx);
    } while (data[0] == '$');
    const std::string str = data.substr(1, data.length() - 1);
    rle_decode_line(str, row, dx);
}

void Canvas::raw_print() const {
    std::stringstream ss;
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int o = 0; o < width; o++) ss << at(i, o) << " ";
        ss << "\n";
    }
    std::cout << ss.str() << std::flush;
}

void Canvas::raw_print_idx() const {
    std::stringstream ss;
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int o = 0; o < width; o++) ss << Utils<unsigned int>::clamped_coords(width, height, i, o) << " ";
        ss << "\n";
    }
    std::cout << ss.str() << std::flush;
}

void Canvas::raw_print_total() const {
    std::stringstream ss;
    for (unsigned int i = 0; i < height; i++) {
        for (unsigned int o = 0; o < width; o++) ss << get_total(i, o) << " ";
        ss << "\n";
    }
    std::cout << ss.str() << std::flush;
}

void Canvas::iter() {
    step_all();
    auto temp = cells;
    cells = buffer;
    buffer = temp;
}


bool Canvas::is_number(char c) {
    return c >= '0' && c <= '9';
}

std::string Canvas::next_token(const std::string &str, unsigned int &offset) {
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

void Canvas::rle_decode_line(const std::string &str, int row, int dx) const {
    std::string number;
    unsigned int ib = dx;
    unsigned int offset = 0;
    std::string next = next_token(str, offset);
    while (offset < str.length()) {
        if (is_number(next[0])) {
            int n = std::stoi(next);
            next = next_token(str, offset);
            generate(ib, row, next == "o", n);
            next = next_token(str, offset);
        } else {
            generate(ib, row, next == "o", 1);
            next = next_token(str, offset);
        }
        if (next == "!") break;
    }
    if (is_number(next[0])) {
        std::cout << "Unexpected number at end of line" << std::endl;
        throw std::runtime_error("Unexpected number at end of line");
    } else if (next != "!") {
        generate(ib, row, next == "o", 1);
    }
}


[[nodiscard]] unsigned int Canvas::get_total(unsigned int x, unsigned int y) const {
    int total = -at_or0(x, y);
    for (int i = -1; i < 2; i++)
        for (int o = -1; o < 2; o++)
            total += at_or0(x + i, y + o);
    return total;
}

[[nodiscard]] unsigned int Canvas::step(unsigned int x, unsigned int y) const {
    unsigned int total = get_total(x, y);
    bool current = at(x, y);
    if (current) {
        if (total < 2) return 0;
        if (total < 4) return 1;
        if (total == 4) return 0;
    }
    if (total == 3) return 1;
    return 0;
}

void Canvas::step_all() const {
    for (unsigned int x = 0; x < width; x++) for (unsigned int y = 0; y < height; y++) buf(x, y) = step(x, y);
}

Canvas::Canvas(unsigned int width, unsigned int height)
        : width(width), height(height) {
    cells = new vector<bitset<BIT_GROUP_SIZE>>;
    buffer = new vector<bitset<BIT_GROUP_SIZE>>;
    cells->reserve(width * height / BIT_GROUP_SIZE + 1);
    buffer->reserve(width * height / BIT_GROUP_SIZE + 1);
    for (unsigned int i = 0; i < width * height / BIT_GROUP_SIZE + 1; i++) {
        cells->emplace_back();
        buffer->emplace_back();
    }
}

Canvas::~Canvas() {
    delete cells;
    delete buffer;
}

void Canvas::generate(unsigned int &ib, unsigned int row, bool value, unsigned int count) const {
    for (unsigned int i = 0; i < count; i++) {
        at(ib++, row) = value;
    }
}
