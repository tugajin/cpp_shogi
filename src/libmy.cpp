#include <cmath>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

#include "libmy.hpp"

std::string timestamp() {
	std::time_t t = std::time(nullptr);
	char mbstr[256];
	if (std::strftime(mbstr, 256, "%Y%m%d-%H%M%S", std::localtime(&t))) {
		std::string str = mbstr;
		return str;
	}
	return "";
}

namespace ml {

	// functions

	// math

	uint64 rand_int_64() {
		static std::mt19937_64 gen;
		return gen();
	}

	int my_rand(int i) {
		return int(rand_int_64() % i);
	}

	int round(double x) {
		return int(floor(x + 0.5));
	}

	int div(int a, int b) {

		assert(b > 0);

		if (b <= 0) {
			Tee << "ml::div(): divide error" << std::endl;
			std::exit(EXIT_FAILURE);
		}

		int div = a / b;
		if (a < 0 && a != b * div) div--; // fix buggy C semantics

		return div;
	}

	int div_round(int a, int b) {
		assert(b > 0);
		return div(a + b / 2, b);
	}

	bool is_power_2(int64 n) {
		assert(n >= 0);
		return (n & (n - 1)) == 0 && n != 0;
	}

	int log_2(int64 n) {

		assert(n > 0);

		int ln = -1;

		for (; n != 0; n >>= 1) {
			ln++;
		}

		assert(ln >= 0);
		return ln;
	}

	std::string trim(const std::string s) {
		std::string str = s;
		const auto pos = str.find(' ');
		if (pos != std::string::npos) {
			str = str.substr(pos + 1);
		}
		return str;
	}

	bool is_exists_file(const std::string path) {
		std::ifstream ifs(path);
		return ifs.is_open();
	}

}