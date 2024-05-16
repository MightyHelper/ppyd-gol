#include <atomic>

#include "canvas.hxx"

using namespace std;
struct Args {
		string file;
		unsigned long long iter_count;
		unsigned long long delay;
};
Args parse(int argc, const char* argv[]){
	switch(argc){
		case 0:
		case 1:
			throw runtime_error("No arguments provided.");
		case 2:
			return Args{argv[1], 0, 50000};
		case 3:
			return Args{argv[1], stoull(argv[2]), 50000};
		case 4:
			return Args{argv[1], stoull(argv[2]), stoull(argv[3])};
	}
	throw runtime_error("Too many arguments provided.");
}
int main(int argc, const char* argv[]) {
	Args args = parse(argc, argv);
	Canvas<2993, 747> canvas{};
//	canvas.init();
//	canvas.spawn(
//		".X..X\n"
//		"X....\n"
//		"X...X\n"
//		"XXXX.\n",
//		0, 0
//	);
//	canvas.print();
	canvas.load_file(args.file);
	cout << CLEAR_SCREEN << flush;
	for (int i = 0; i < args.iter_count; i++) {
		cout << GOTO_0_0;
		canvas.print();
		canvas.iter();
		usleep(args.delay);
	}
	cout << GOTO_0_0;
	canvas.print();
}
