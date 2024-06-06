#include "../include/canvas.h"
#include "../include/arg_parse.h"
#include <chrono>
#include <iostream>

using namespace std;
using namespace chrono;

/*
 * Performs iterations on the Canvas object for a specified amount of time (in milliseconds).
 * It measures the time taken for each iteration and the total time taken for all iterations.
 * The function returns an OutputValues object containing the number of iterations and the
 * total time taken.
 * */
OutputValues time_based(Canvas &canvas, long millis) {
	auto start = high_resolution_clock::now();
	unsigned long long its = 0;
	unsigned long long compute_time = 0;
	while (true) {
		auto comm_start = high_resolution_clock::now();
		canvas.iter();
		auto end = high_resolution_clock::now();
		auto duration = duration_cast<milliseconds>(end - start);
		its++;
		compute_time += duration_cast<nanoseconds>(end - comm_start).count();
		if (duration.count() >= millis) break;
	}
	auto end = high_resolution_clock::now();
	auto duration = duration_cast<nanoseconds>(end - start);
	return OutputValues{
			0, its, (unsigned long long) duration.count(),
			0, 0, compute_time
	};
}

/*
 * Performs a specified number of iterations on the Canvas object.
 * Similar to the time_based function, it measures the time taken for each iteration and the total
 * time taken for all iterations. The function returns an OutputValues object containing the number
 * of iterations and the total time taken.
 * */
OutputValues iter_based(Canvas &canvas, unsigned long long iter_count) {
	auto start = high_resolution_clock::now();
	unsigned long long compute_time = 0;
	for (unsigned long long i = 0; i < iter_count; i++) {
		auto compute_start = high_resolution_clock::now();
		canvas.iter();
		auto end = high_resolution_clock::now();
		compute_time += duration_cast<nanoseconds>(end - compute_start).count();
	}
	auto end = high_resolution_clock::now();
	auto duration = duration_cast<nanoseconds>(end - start);
	return OutputValues{
			0, iter_count, (unsigned long long) duration.count(),
			0, 0, compute_time
	};
}

/*
 * Parses command-line arguments, creates a Canvas object, loads a file into the Canvas,
 * and then performs either time-based or iteration-based computations on the Canvas depending
 * on the parsed arguments. The results are then printed to the console.
 * */
void program_main(int argc, char **argv) {
	Args args = Args::parse(argc, argv, true);
	cout << args;
	Canvas canvas{args.width, args.height};
	canvas.load_file(args.file, 0, 0);
	OutputValues ov = args.type ? time_based(canvas, args.millis) : iter_based(canvas, args.iter_count);
	cout << "output: \n" << ov;
}

/*
 * Calls the program_main function and handles any exceptions that might be thrown.
 * */
int main(int argc, char *argv[]) {
	try {
		program_main(argc, argv);
		return 0;
	} catch (const exception &e) {
		cerr << e.what() << endl;
		return 1;
	}
}
