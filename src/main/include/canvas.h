#ifndef GOL_CANVAS_H
#define GOL_CANVAS_H
#define BIT_GROUP_SIZE 3200

#include <bitset>
#include <vector>
#include <functional>

class Canvas {
public:
    unsigned int width, height;
    std::vector<std::bitset<BIT_GROUP_SIZE>> *cells = nullptr;
    std::vector<std::bitset<BIT_GROUP_SIZE>> *buffer = nullptr;

    [[nodiscard]] std::bitset<BIT_GROUP_SIZE>::reference
    index(std::vector<std::bitset<BIT_GROUP_SIZE>> *mat, unsigned int index) const;

    [[nodiscard]] std::bitset<BIT_GROUP_SIZE>::reference at(unsigned int x, unsigned int y) const;

    [[nodiscard]] int at_or0(unsigned int x, unsigned int y) const;

    [[nodiscard]] std::bitset<BIT_GROUP_SIZE>::reference buf(unsigned int x, unsigned int y) const;

    void print() const;

    void init();

    void spawn(std::string &data, int ox, int oy) const;

    void load_file(const std::string &str, int dx = 0, int dy = 0);

    void load_file(const char *path, int dx = 0, int dy = 0);

    void raw_print() const;

    void raw_print_idx() const;

    void raw_print_total() const;

    void iter();

    [[nodiscard]] unsigned int get_total(unsigned int x, unsigned int y) const;

    [[nodiscard]] unsigned int step(unsigned int x, unsigned int y) const;

    void step_all() const;

    Canvas(unsigned int width, unsigned int height);

    ~Canvas();

    Canvas(const Canvas&) = delete;

};

#endif //GOL_CANVAS_H
