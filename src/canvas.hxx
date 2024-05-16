//
// Created by federico on 5/9/24.
//

#ifndef GOL_CANVAS_HXX
#define GOL_CANVAS_HXX

#include <bitset>
#include "utils.hxx"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#define cchar(i) center[i*3] << center[i*3+1] << center[i*3+2]
#define bchar(i) box[i*3] << box[i*3+1] << box[i*3+2]
//const char *center = "▒█";
const char *center = "░█";
const char *box = "┌─┐│█│└─┘";
#define CLEAR_SCREEN "\033[2J"
#define GOTO_0_0 "\033[0;0H"

#include "utils.hxx"

template<int width, int height>
class Canvas {
public:
		std::bitset<width * height> *cells;
		std::bitset<width * height> *buffer;

		std::bitset<width * height>::reference at(int x, int y) const {
			return (*cells)[clamped_coords<width, height>(x, y)];
		}

		[[nodiscard]] int at_or0(int x, int y) const {
			if (x < 0 || x >= width || y < 0 || y >= height) return 0;
			return (*cells)[clamped_coords<width, height>(x, y)];
		}

		std::bitset<width * height>::reference buf(int x, int y) const {
			return (*buffer)[clamped_coords<width, height>(x, y)];
		}

		void print() const {
			std::stringstream ss;
			ss << bchar(0);
			for (int o = 0; o < width; o++) ss << bchar(1) << bchar(1);
			ss << bchar(2);
			ss << "\n";
			for (int i = 0; i < height; i++) {
				ss << bchar(3);
				for (int o = 0; o < width; o++) ss << cchar(at(o, i)) << cchar(at(o, i));
				ss << bchar(5) << "\n";
			}
			ss << bchar(6);
			for (int o = 0; o < width; o++)ss << bchar(7) << bchar(7);

			ss << bchar(8) << "\n";
			std::cout << ss.str() << std::flush;
		}

		void init() {
			cells = new std::bitset<width * height>;
			buffer = new std::bitset<width * height>;
			for (int i = 0; i < height * width; i++) {
				(*cells)[i] = 0;
				(*buffer)[i] = 0;
			}
		}

		void spawn(std::string data, int ox, int oy) {
			int x = ox;
			int y = oy;
			for (int i = 0; i < data.length(); i++) {
				switch (data[i]) {
					case '\n':
						x = ox;
						y++;
						break;
					case '.':
						at(x, y) = 0;
						x++;
						break;
					case 'X':
						at(x, y) = 1;
						x++;
				}
			}
		}

		void load_file(std::string str) {
			load_file(str.c_str());
		}

		void load_file(const char *path) {
			std::ifstream f(path);
			std::stringstream buf;
			if (!f) throw std::runtime_error(std::string("File ") + path + " not found.");
			buf << f.rdbuf();
			std::string data = buf.str();
			init();
			while (data[0] == '#') {
				data = data.substr(data.find('\n') + 1);
			}
			data = data.substr(data.find('\n') + 1);
			data.erase(std::remove(data.begin(), data.end(), '\n'), data.end());
			int row = 0;
			do {
				unsigned long idx = data.find('$', 1);
				if (idx == std::string::npos) break;
				const std::string str = data.substr(1, idx - 1);
				rle_decode_line(str, row++);
				data = data.substr(idx);
			} while (data[0] == '$');
			const std::string str = data.substr(1, data.length() - 1);
			rle_decode_line(str, row);
		}

		void raw_print() const {
			std::stringstream ss;
			for (int i = 0; i < height; i++) {
				for (int o = 0; o < width; o++) ss << at(i, o) << " ";
				ss << "\n";
			}
			std::cout << ss.str() << std::flush;
		}

		void raw_print_idx() const {
			std::stringstream ss;
			for (int i = 0; i < height; i++) {
				for (int o = 0; o < width; o++) ss << clamped_coords<width, height>(i, o) << " ";
				ss << "\n";
			}
			std::cout << ss.str() << std::flush;
		}

		void raw_print_total() const {
			std::stringstream ss;
			for (int i = 0; i < height; i++) {
				for (int o = 0; o < width; o++) ss << get_total(i, o) << " ";
				ss << "\n";
			}
			std::cout << ss.str() << std::flush;
		}

		void iter() {
			step_all();
			auto temp = cells;
			cells = buffer;
			buffer = temp;
		}


		bool is_number(char c) {
			return c >= '0' && c <= '9';
		}

		std::string next_token(const std::string &str, int &offset) {
			// if number, read until non-number
			// if letter just return letter
			std::string out;
			if (offset >= str.length()) return out;
			if (is_number(str[offset])) {
				while (is_number(str[offset])) {
					out += str[offset++];
				}
			} else {
				out = str[offset++];
			}
			return out;
		}

		void rle_decode_line(const std::string &str, int row) {
			std::string number;
			int ib = 0;
			int offset = 0;
			std::string next = next_token(str, offset);
			while (offset < str.length()) {
				if (is_number(next[0])) {
					int n = std::stoi(next);
					next = next_token(str, offset);
					generate(ib, row, next == "o", n);
					next = next_token(str, offset);
				} else {
					generate(ib, row, next == "o", 1);
					next = next_token(str, offset);
				}
				if (next == "!") break;
			}
			if (is_number(next[0])) {
				std::cout << "Unexpected number at end of line" << std::endl;
				throw std::runtime_error("Unexpected number at end of line");
			} else if (next != "!") {
				generate(ib, row, next == "o", 1);
			}
		}


		int get_total(int x, int y) {
			int total = -at_or0(x, y);
			for (int i = -1; i < 2; i++)
				for (int o = -1; o < 2; o++)
					total += at_or0(x + i, y + o);
			return total;
		}

		int step(int x, int y) {
			int total = get_total(x, y);
			int current = at(x, y);
			if (current) {
				if (total < 2) return 0;
				if (total < 4) return 1;
				if (total == 4) return 0;
			}
			if (total == 3) return 1;
			return 0;
		}

		void step_all() {
			for (int x = 0; x < width; x++) for (int y = 0; y < height; y++) buf(x, y) = step(x, y);
		}

		Canvas() = default;

		void generate(int &ib, int row, bool value, int count) {
			for (int i = 0; i < count; i++) {
				at(ib++, row) = value;
			}
		}
};


#endif //GOL_CANVAS_HXX
