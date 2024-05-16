#include <atomic>

#include "canvas.hxx"
#define S_WIDTH 154
#define S_HEIGHT 94

using namespace std;
struct Args {
		string file;
		unsigned long long iter_count;
		unsigned long long delay;
};

void program_main(int argc, const char **argv);

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
int main(int argc, const char* argv[]) {
	try{
		program_main(argc, argv);
		return 0;
	} catch (const exception &e) {
		cerr << e.what() << endl;
		return 1;
	}
}

void program_main(int argc, const char **argv) {
	Args args = parse(argc, argv);
	Canvas<S_WIDTH, S_HEIGHT> canvas{};
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
