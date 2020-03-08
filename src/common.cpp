#include "common.hpp"
#include "bit.hpp"

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

namespace common {

	void init() {
		auto index = 0;
		SQUARE_FOREACH(from) {
			SQUARE_FOREACH(to) {
				gDirection[from][to] = DIR_NONE;
				//check knight
				auto bb = bit::get_knight_attack(BLACK, from);
				if (bb.is_set(to)) {
					if (from > to) {
						gDirection[from][to] = DIR_R_KNT;
					}
					else {
						gDirection[from][to] = DIR_L_KNT;
					}
				}
				//check bishop
				bit::Bitboard occ;
				occ.init();
				bb = bit::get_diag1_attack(from, occ);
				if (bb.is_set(to)) {
					if (from > to) {
						gDirection[from][to] = DIR_DR;
					}
					else {
						gDirection[from][to] = DIR_UL;
					}
				}
				bb = bit::get_diag2_attack(from, occ);
				if (bb.is_set(to)) {
					if (from > to) {
						gDirection[from][to] = DIR_UR;
					}
					else {
						gDirection[from][to] = DIR_DL;
					}
				}
				//check rook
				bb = bit::get_file_attack(from, occ);
				if (bb.is_set(to)) {
					if (from > to) {
						gDirection[from][to] = DIR_UP;
					}
					else {
						gDirection[from][to] = DIR_DW;
					}
				}
				bb = bit::get_rank_attack(from, occ);
				if (bb.is_set(to)) {
					if (from > to) {
						gDirection[from][to] = DIR_RG;
					}
					else {
						gDirection[from][to] = DIR_LF;
					}
				}
			}
		}
	}

}