#include <iostream>
#include <string>
#include <iterator>

#include <unistd.h>
#include <sstream>
#include <fstream>

#define CLEAR_SCREEN "\033[2J"
#define GOTO_0_0 "\033[0;0H"
#define SIM_WIDTH 10
#define SIM_HEIGHT 10
#define coords(x, y) (x * SIM_WIDTH + y)
#define clamp(v, m, M) (v < m ? m : (v > M ? M : v))
#define clamped_coords(x, y) coords(clamp(x, 0, SIM_WIDTH - 1), clamp(y, 0, SIM_HEIGHT - 1))
using namespace std;

int *cells;
int *buffer;

int get_total(int x, int y) {
    int total = 0;
    for (int i = -1; i < 2; i++)
        for (int o = -1; o < 2; o++)
            total += cells[clamped_coords(x + i, y + o)];
    return total - cells[clamped_coords(x, y)];
}

int step(int x, int y) {
    int total = get_total(x, y);
    int current = cells[clamped_coords(x, y)];
    // cout << x << " " << y << " " << total << " " << current << endl;
    if (current) {
        if (total < 2) return 0;
        if (total < 4) return 1;
        if (total == 4) return 0;
    }
    if (total == 3) return 1;
    return 0;
}

void step_all() {
    for (int i = 0; i < SIM_HEIGHT; i++) {
        for (int o = 0; o < SIM_WIDTH; o++) {
            buffer[clamped_coords(i, o)] = step(i, o);
        }
    }
}

const char *center = "▒█";
const char *box = "┌─┐│█│└─┘";

string &advance_block(int x, int y, char last_type, string &count);

#define cchar(i) center[i*3] << center[i*3+1] << center[i*3+2]
#define bchar(i) box[i*3] << box[i*3+1] << box[i*3+2]

void print() {
    stringstream ss;
    ss << bchar(0);
    for (int o = 0; o < SIM_WIDTH; o++) {
        ss << bchar(1) << bchar(1);
    }
    ss << bchar(2);
    ss << "\n";
    for (int i = 0; i < SIM_HEIGHT; i++) {
        ss << bchar(3);
        for (int o = 0; o < SIM_WIDTH; o++) {
            ss << cchar(cells[clamped_coords(i, o)]) << cchar(cells[clamped_coords(i, o)]);
        }
        ss << bchar(5) << "\n";
    }
    ss << bchar(6);
    for (int o = 0; o < SIM_WIDTH; o++) {
        ss << bchar(7) << bchar(7);
    }
    ss << bchar(8) << "\n";
    string x = ss.str();
    cout << x << flush;
}

void raw_print() {
    for (int i = 0; i < SIM_HEIGHT; i++) {
        for (int o = 0; o < SIM_WIDTH; o++) {
            cout << cells[clamped_coords(i, o)] << " ";
        }
        cout << "\n";
    }
}

void raw_print_idx() {
    for (int i = 0; i < SIM_HEIGHT; i++) {
        for (int o = 0; o < SIM_WIDTH; o++) {
            cout << clamped_coords(i, o) << " ";
        }
        cout << "\n";
    }
}

void raw_print_total() {
    for (int i = 0; i < SIM_HEIGHT; i++) {
        for (int o = 0; o < SIM_WIDTH; o++) {
            cout << get_total(i, o) << " ";
        }
        cout << "\n";
    }
}

void init() {
    cells = new int[SIM_HEIGHT * SIM_WIDTH];
    buffer = new int[SIM_HEIGHT * SIM_WIDTH];
    for (int i = 0; i < SIM_HEIGHT; i++) {
        for (int o = 0; o < SIM_WIDTH; o++) {
            cells[clamped_coords(i, o)] = 0;
            buffer[clamped_coords(i, o)] = 0;
        }
    }
}

void spawn(string data, int ox, int oy) {
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

void iter() {
    step_all();
    void *temp = cells;
    cells = buffer;
    buffer = (int *) temp;
}

void load_file(const char *path) {
    ifstream f(path);
    if (!f){
        cerr << "File " << path << " not found." << endl;
        exit(-1);
    }
    string data;
    string header;
    string meta;
    string header_line;
    getline(f, header_line);
    while (header_line[0] == '#') {
        header += header_line + "\n";
        getline(f, header_line);
    }
    meta = header_line;
    init();
    getline(f, data, '!');
    int x = 0;
    int y = 0;
    char last_type = 'z';
    cout << data << endl;
    string count;
    for (int i=0; i<data.length(); i++) {
        char c = data[i];
        switch (c) {
            case '$':
                count = advance_block(x, y, last_type, count);
                x = 0;
                y++;
                break;
            case 'b':
                count = advance_block(x, y, last_type, count);
                last_type = 'b';
                break;
            case 'o':
                count = advance_block(x, y, last_type, count);
                last_type = 'o';
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                count += c;
        }
    }
    // cout << header << "\n" << endl;
    // cout << meta << "\n" << endl;
    // cout << data << endl;
}

string &advance_block(int x, int y, char last_type, string &count) {
    int i;
    if (!count.empty()) {
        i = atoi(count.c_str());
    }else{
        i = 1;
    }
    cout << i << "*" << last_type << endl;
    int cnt = x + i;
    for (; x < cnt; x++) {
        cells[clamped_coords(x, y)] = last_type == 'b';
    }
    count = "";
    return count;
}

int main() {
    // cout << CLEAR_SCREEN;
    // init();
    // spawn( 
    //     ".X..X\n"
    //     "X....\n"
    //     "X...X\n"
    //     "XXXX.\n",
    //     16, 16
    // );
    // for (int i=0; i<100; i++){
    //     cout << GOTO_0_0;
    //     print();
    //     iter();
    //     usleep(100000);
    // }
    // load_file("c2-orthogonal.rle");   
    load_file("mini.rle");
    print();
}