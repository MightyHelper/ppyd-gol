#include <atomic>
#include <chrono>

#include "canvas.hxx"
#define S_WIDTH 240
#define S_HEIGHT 120
#define PRINT false

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
		default:
			throw runtime_error("Too many arguments provided.");
	}
}
void program_main(int argc, const char **argv) {
	Args args = parse(argc, argv);
	Canvas<S_WIDTH, S_HEIGHT> canvas{};
	canvas.load_file(args.file, 0, 0);
	if (PRINT) cout << CLEAR_SCREEN << flush;
	double its = 0;
	// use chrono to get the current time
	auto start = chrono::high_resolution_clock::now();
	for (int i = 0; i < args.iter_count; i++) {
		if (PRINT) cout << GOTO_0_0;
		if (PRINT) canvas.print();
		canvas.iter();
		usleep(args.delay);
		if (i % 10 == 0) {
			auto end = chrono::high_resolution_clock::now();
			auto duration = chrono::duration_cast<chrono::nanoseconds>(end - start);
			start = chrono::high_resolution_clock::now();
			its = 1000000000.0 / duration.count();
		}
	}
	cout << "Iterations per second: " << its << endl;
	if (PRINT){
		cout << GOTO_0_0;
		canvas.print();
	}
}

int main(int argc, const char* argv[]) {
	try{
		program_main(argc, argv);
		return 0;
	} catch (const exception &e) {
		cerr << e.what() << endl;
		return 1;
	}
}
