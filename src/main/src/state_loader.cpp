#include <iostream>
#include <sstream>
#include "../include/state_loader.h"
#include "../include/canvas.h"

/*
 * StateLoader::is_number function: This function checks if a character is a number.
 * */
bool StateLoader::is_number(char c) {
	return c >= '0' && c <= '9';
}

/*
 * StateLoader::next_token function: This function reads the next token from a string. If the token is a number, it reads
 * all consecutive digits. If the token is a letter, it just returns the letter.
 * */
std::string StateLoader::next_token(const std::string &str, unsigned int &offset) {
	// if number, read until non-number
	// if letter just return letter
	std::string out;
	if (offset >= str.length()) return out;
	if (StateLoader::is_number(str[offset])) {
		while (StateLoader::is_number(str[offset])) {
			out += str[offset++];
		}
	} else {
		out = str[offset++];
	}
	return out;
}

/*
* StateLoader::rle_decode_line function: This function decodes a line of RLE data. It reads tokens from the line and
* generates cells based on the tokens. The spawn parameter is a function that sets the state of a cell at a given position.
*/
void
StateLoader::rle_decode_line(const std::string &str, const std::function<void(unsigned int, unsigned int, bool)> &spawn,
                             int row, int dx) {
	std::string number;
	unsigned int ib = dx;
	unsigned int offset = 0;
	std::string next = next_token(str, offset);
	while (offset < str.length()) {
		if (StateLoader::is_number(next[0])) {
			int n = std::stoi(next);
			next = next_token(str, offset);
			generate(ib, row, next == "o", n, spawn);
			next = next_token(str, offset);
		} else {
			generate(ib, row, next == "o", 1, spawn);
			next = next_token(str, offset);
		}
		if (next == "!") break;
	}
	if (StateLoader::is_number(next[0])) {
		std::cout << "Unexpected number at end of line" << std::endl;
		throw std::runtime_error("Unexpected number at end of line");
	} else if (next != "!") {
		generate(ib, row, next == "o", 1, spawn);
	}
}

/*
 * StateLoader::generate function: This function generates a number of cells with a given state. The spawn parameter is a
 * function that sets the state of a cell at a given position.
 * */
void StateLoader::generate(
		unsigned int &ib,
		unsigned int row,
		bool value,
		unsigned int count,
		const std::function<void(unsigned int, unsigned int, bool)> &spawn
) {
	for (unsigned int i = 0; i < count; i++) spawn(ib++, row, value);
}

/*
* StateLoader::load_from_file function: This function loads a state from a file and applies it to the canvas. It reads
* lines of RLE data from the file and decodes them using the rle_decode_line function.
*/
void StateLoader::load_from_file(const char *path, int dx, int dy,
                                 const std::function<void(unsigned int, unsigned int, bool)> &spawn) {
	std::ifstream f(path);
	std::stringstream buf;
	if (!f) throw std::runtime_error(std::string("File ") + path + " not found.");
	buf << f.rdbuf();
	std::string data = buf.str();
	while (data[0] == '#') data = data.substr(data.find('\n') + 1);
	data = data.substr(data.find('\n') + 1);
	data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());
	int row = dy;
	do {
		unsigned long idx = data.find('$', 1);
		if (idx == std::string::npos) break;
		const std::string str = data.substr(1, idx - 1);
		StateLoader::rle_decode_line(str, spawn, row++, dx);
		data = data.substr(idx);
	} while (data[0] == '$');
	const std::string str = data.substr(1, data.length() - 1);
	StateLoader::rle_decode_line(str, spawn, row, dx);
}
