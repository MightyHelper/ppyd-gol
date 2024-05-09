//
// Created by federico on 5/8/24.
//

#ifndef GOL_GOLIO_H
#define GOL_GOLIO_H

#include <bitset>

template<int width, int height>
struct State {
    std::bitset<width * height> *cells;
    std::bitset<width * height> *buffer;

    State();

    std::bitset<width * height>::reference at(int x, int y) const;

    void print() const;

    void raw_print() const;

    void raw_print_idx() const;

    void raw_print_total() const;

    void init();

    void spawn(std::string data, int ox, int oy);

    void iter();

    void load_file(const char *path);

    std::string &advance_block(int x, int y, char last_type, std::string &count);

    int get_total(int x, int y);

    int step(int x, int y);

    void step_all();
};

#endif //GOL_GOLIO_H
