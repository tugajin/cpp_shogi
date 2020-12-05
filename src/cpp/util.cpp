#include <string>
#include "libmy.hpp"
#include "util.hpp"

int find(char c, const std::string& s) {
	auto i = s.find(c);
	if (i == std::string::npos) { throw BadInput(); }
	return int(i);
}