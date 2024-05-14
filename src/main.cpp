#include <atomic>

#include "canvas.hxx"

using namespace std;

int main() {
    // load_file("c2-orthogonal.rle");
    Canvas<9, 9> canvas{};
    canvas.init();
    canvas.spawn(
            ".X..X\n"
            "X....\n"
            "X...X\n"
            "XXXX.\n",
            0, 0
    );
    canvas.print();
//    canvas.load_file("../data/mini.rle");
//    cout << CLEAR_SCREEN << flush;
//    for (int i=0; i<100; i++){
//         cout << GOTO_0_0;
//         canvas.print();
//         canvas.iter();
//         usleep(50000);
//     }
//    canvas.generate(lb, 0, true, 4);
//    canvas.generate(lb, 0, false, 1);
//    canvas.generate(lb, 0, true, 1);
    canvas.print();
}
