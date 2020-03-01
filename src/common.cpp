#include "common.hpp"

Square sq_from_string(const std::string& s) {
	assert(s.length() == 2);
	return square_make(File(s[0] - '1'), Rank(s[1] - 'a'));
}
std::string sq_to_string(const Square sq) {
	std::string s;
	s += '1' + square_file(sq);
	s += 'a' + square_rank(sq);
	return s;
}