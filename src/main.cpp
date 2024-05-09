#include <iostream>
#include <string>
#include <iterator>

#include <unistd.h>
#include <sstream>
#include <fstream>
#include "golio.h"
#include "utils.h"

#define CLEAR_SCREEN "\033[2J"
#define GOTO_0_0 "\033[0;0H"
#define SIM_WIDTH 10
#define SIM_HEIGHT 10
using namespace std;



string &advance_block(int x, int y, char last_type, string &count);

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
    State<1000,1000> state;
    state.load_file("mini.rle");
    state.print();
}