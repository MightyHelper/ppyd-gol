#include <string>

#include "golio.h"

using namespace std;
#define SIM_WIDTH 10
#define SIM_HEIGHT 10



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
//    cout << "Hi" <<endl;
    State<1000,1000> state;
    state.load_file("mini.rle");
    state.print();
}