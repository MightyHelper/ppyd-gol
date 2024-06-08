#include "../include/canvas.h"
#include "../include/utils.h"
#include "../include/state_loader.h"
#include <sstream>
#include <iostream>

#define cchar(i) center[i*3] << center[i*3+1] << center[i*3+2]
#define bchar(i) box[i*3] << box[i*3+1] << box[i*3+2]
//const char *center = "▒█";
const char *center = "░█";
const char *box = "┌─┐│█│└─┘";

using namespace std;

/*
 * Returns a reference to a bit in a bitset at a given index. The bitset is part of a vector of bitsets,
 * and the index is used to locate the correct bitset and the bit within that bitset.
 * */
[[nodiscard]] bitset<BIT_GROUP_SIZE>::reference
Canvas::index(vector<bitset<BIT_GROUP_SIZE>> *mat, unsigned int index) const {
	return (*mat)[index / BIT_GROUP_SIZE][index % BIT_GROUP_SIZE];
}

/*
 * Canvas::at and Canvas::at_or0 return the state of a cell at a given position on the canvas. The at
 * function throws an exception if the position is out of bounds, while the at_or0 function returns 0 in
 * that case.
 */
[[nodiscard]] bitset<BIT_GROUP_SIZE>::reference Canvas::at(unsigned int x, unsigned int y) const {
	return index(cells, MathUtils<unsigned int>::clamped_coords(width, height, x, y));
}

[[nodiscard]] int Canvas::at_or0(unsigned int x, unsigned int y) const {
	if (x >= width || y >= height) return 0;
	return index(cells, MathUtils<unsigned int>::clamped_coords(width, height, x, y));
}

[[nodiscard]] std::bitset<BIT_GROUP_SIZE>::reference Canvas::buf(unsigned int x, unsigned int y) const {
	return index(buffer, MathUtils<unsigned int>::clamped_coords(width, height, x, y));
}

/*
 * Canvas::print function: This function prints the current state of the canvas to the console.
 * The state is represented as a grid of characters, with different characters representing live and dead
 * cells.
 * */
void Canvas::print() const {
	std::stringstream ss;
	ss << bchar(0);
	for (unsigned int o = 0; o < width; o++) ss << bchar(1) << bchar(1);
	ss << bchar(2);
	ss << "\n";
	for (unsigned int i = 0; i < height; i++) {
		ss << bchar(3);
		for (unsigned int o = 0; o < width; o++) ss << cchar(at(o, i)) << cchar(at(o, i));
		ss << bchar(5) << "\n";
	}
	ss << bchar(6);
	for (unsigned int o = 0; o < width; o++)ss << bchar(7) << bchar(7);

	ss << bchar(8) << "\n";
	std::cout << ss.str() << std::flush;
}

/*
 * Canvas::init function: This function initializes the canvas and the buffer by setting all cells
 * to dead.
 * */
void Canvas::init() {
	for (unsigned int i = 0; i < cells->size(); i++) {
		(*cells)[i] = 0;
		(*buffer)[i] = 0;
	}
	for (unsigned int i = 0; i < height * width; i++) {
		index(cells, i) = false;
		index(buffer, i) = false;
	}
}

/*
 * Canvas::spawn function: This function sets the state of the cells in a given area of the canvas
 * according to a string of characters. The characters '.' and 'X' represent dead and live cells,
 * respectively.
 * */
void Canvas::spawn(std::string &data, int ox, int oy) const {
	int x = ox;
	int y = oy;
	for (unsigned int i = 0; i < data.length(); i++) {
		switch (data[i]) {
			case '\n':
				x = ox;
				y++;
				break;
			case '.':
				at(x, y) = false;
				x++;
				break;
			case 'X':
				at(x, y) = true;
				x++;
		}
	}
}

/*
 * Canvas::load_file function: This function loads a state from a file and applies it to the canvas.
 * The state is represented as a grid of characters, similar to the spawn function.
 * */
void Canvas::load_file(const std::string &str, int dx, int dy) {
	load_file(str.c_str(), dx, dy);
}

void Canvas::load_file(const char *path, int dx, int dy) {
  const std::function<void(unsigned int, unsigned int, bool)> spawn = [this](unsigned int x, unsigned int y,
                                                                             bool value) {
      at(x, y) = value;
  };
  StateLoader::load_from_file(path, dx, dy, spawn);
}


void Canvas::raw_print() const {
	std::stringstream ss;
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int o = 0; o < width; o++) ss << at(i, o) << " ";
		ss << "\n";
	}
	std::cout << ss.str() << std::flush;
}

void Canvas::raw_print_idx() const {
	std::stringstream ss;
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int o = 0; o < width; o++)
			ss << MathUtils<unsigned int>::clamped_coords(width, height, i, o) << " ";
		ss << "\n";
	}
	std::cout << ss.str() << std::flush;
}

void Canvas::raw_print_total() const {
	std::stringstream ss;
	for (unsigned int i = 0; i < height; i++) {
		for (unsigned int o = 0; o < width; o++) ss << get_total(i, o) << " ";
		ss << "\n";
	}
	std::cout << ss.str() << std::flush;
}

/*
 * Canvas::iter function: This function performs one iteration of the cellular automaton. It calculates
 * the next state of each cell and stores it in the buffer, then swaps the buffer with the current state.
 * */
void Canvas::iter() {
	step_all();
	auto temp = cells;
	cells = buffer;
	buffer = temp;
}

void Canvas::iter2() {
  step_all();
  auto temp = cells;
  cells = buffer;
  buffer = temp;
}

/*
 * Canvas::get_total function: This function calculates the total number of live neighbors of a cell.
 * This total is used to determine the next state of the cell according to the rules of the cellular
 * automaton.
*/
[[nodiscard]] unsigned int Canvas::get_total(unsigned int x, unsigned int y) const {
	int total = -at_or0(x, y);
	for (int i = -1; i < 2; i++)
		for (int o = -1; o < 2; o++)
			total += at_or0(x + i, y + o);
	return total;
}

//[[nodiscard]] unsigned int Canvas::get_total2(unsigned int x, unsigned int y) {
//  auto idx = MathUtils<unsigned int>::coords(width, x-1, y-1);
//  auto idx2 = MathUtils<unsigned int>::coords(width, x-1, y+0);
//  auto idx3 = MathUtils<unsigned int>::coords(width, x-1, y+1);
//  auto a = index(cells, idx+0);
//  auto b = index(cells, idx+1);
//  auto c = index(cells, idx+2);
//  auto d = index(cells, idx2+0);
//  auto e = index(cells, idx2+2);
//  auto f = index(cells, idx3+0);
//  auto g = index(cells, idx3+1);
//  auto h = index(cells, idx3+2);
//  return a+b+c+d+e+f+g+h;
//}

/* Canvas::step function: This function calculates the next state of a cell based on its current state
 * and the total number of live neighbors.
 */
[[nodiscard]] unsigned int Canvas::step(unsigned int x, unsigned int y) const {
	unsigned int total = get_total(x, y);
	bool current = at(x, y);
	if (current) {
		if (total < 2) return 0;
		if (total < 4) return 1;
		if (total == 4) return 0;
	}
	if (total == 3) return 1;
	return 0;
}

/*
 * Canvas::step_all function: This function calculates the next state of all cells and stores it in
 * the buffer.
 */
void Canvas::step_all() const {
	for (unsigned int x = 0; x < width; x++) for (unsigned int y = 0; y < height; y++) buf(x, y) = step(x, y);
}

void Canvas::step_all2() const {
  for (unsigned int x = 1; x < width - 1; x++) for (unsigned int y = 1; y < height - 2; y++) buf(x, y) = step(x, y);
}

/*
 * Canvas constructor: This function initializes the canvas with the specified width and height. It also
 * initializes the cells and buffer vectors.
 * */
Canvas::Canvas(unsigned int width, unsigned int height)
  : width(width), height(height) {
  cells = new vector<bitset<BIT_GROUP_SIZE>>;
  buffer = new vector<bitset<BIT_GROUP_SIZE>>;
  cells->reserve(width * height / BIT_GROUP_SIZE + 1);
  buffer->reserve(width * height / BIT_GROUP_SIZE + 1);
  for (unsigned int i = 0; i < width * height / BIT_GROUP_SIZE + 1; i++) {
    cells->emplace_back();
    buffer->emplace_back();
  }
}

/*
 * Canvas destructor: This function deletes the cells and buffer vectors.
 * */
Canvas::~Canvas() {
	delete cells;
	delete buffer;
}

