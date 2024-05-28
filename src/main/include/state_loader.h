#ifndef GOL_STATE_LOADER_H
#define GOL_STATE_LOADER_H


#include <vector>
#include <bitset>
#include <functional>
#include <string>
#include <fstream>

class StateLoader {
public:
    static bool is_number(char c);

    static std::string next_token(const std::string &str, unsigned int &offset);

    static void
    rle_decode_line(const std::string &str, const std::function<void(unsigned int, unsigned int, bool)> &spawn, int row,
                    int dx = 0);

    static void generate(unsigned int &ib, unsigned int row, bool value, unsigned int count,
                         const std::function<void(unsigned int, unsigned int, bool)> &spawn);

    static void load_from_file(const char *path, int dx, int dy,
                               const std::function<void(unsigned int, unsigned int, bool)> &spawn);
};


#endif //GOL_STATE_LOADER_H
