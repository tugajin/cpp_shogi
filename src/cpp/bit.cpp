#include "bit.hpp"
#include <array>
#include <bitset>

namespace bit {

	std::array<Bitboard, SQUARE_SIZE> g_mask;
	std::array<Bitboard, SQUARE_SIZE> g_diag1_mask;
	std::array<Bitboard, SQUARE_SIZE> g_diag2_mask;
	std::array<Bitboard, FILE_SIZE> g_file_mask;
	std::array<Bitboard, RANK_SIZE> g_rank_mask;

	std::array<Bitboard, SIDE_SIZE> g_prom; //1~3
	std::array<Bitboard, SIDE_SIZE> g_middle; //4~9
	bit::Bitboard g_all_one_bb;
	bit::Bitboard g_all_zero_bb;
	std::array<std::array<bit::Bitboard, 1 << 9>, SIDE_SIZE> g_double_pawn_mask;

	bw_square_t g_knight_attacks;
	bw_square_t g_silver_attacks;
	bw_square_t g_gold_attacks;

	std::array<std::array<uint64, 128>, FILE_SIZE> g_file_attack;
	std::array<std::array<bit::Bitboard, 128>, RANK_SIZE> g_rank_attack;
	std::array<bit::Bitboard, 1856 + 1> g_diag1_attack;
	std::array<bit::Bitboard, 1856 + 1> g_diag2_attack;
	std::array<int, SQUARE_SIZE> g_diag1_offset;
	std::array<int, SQUARE_SIZE> g_diag2_offset;

	bw_square_t g_lance_mask;

	Bitboard gBetween[638];
	Bitboard gBeyond[393];

	int gBetweenIndex[SQUARE_SIZE][SQUARE_SIZE];
	int gBeyondIndex[SQUARE_SIZE][SQUARE_SIZE];

	bit::Bitboard g_check_area[SIDE_SIZE][PIECE_SIZE][SQUARE_SIZE];


	static void valid_set(bit::Bitboard& bb, File f, Rank r) {

		if (is_valid(f, r)) {
			auto sq = square_make(f, r);
			bb |= sq;
		}
	}

	static bit::Bitboard init_knight_attacks(Side sd, File f, Rank r) {

		bit::Bitboard bb;
		bb.init();

		auto opposit = (sd == BLACK) ? 1 : -1;
		auto file = f + (-1);
		auto rank = r + (-1) * 2 * opposit;
		valid_set(bb, file, rank);

		file = f + 1;
		rank = r + (-1) * 2 * opposit;
		valid_set(bb, file, rank);

		return bb;
	}
	static bit::Bitboard init_silver_attacks(Side sd, File f, Rank r) {

		bit::Bitboard bb;
		bb.init();

		auto opposit = (sd == BLACK) ? 1 : -1;
		auto file = f + (-1);
		auto rank = r + (-1) * opposit;
		valid_set(bb, file, rank);

		file = f;
		rank = r + (-1) * opposit;
		valid_set(bb, file, rank);

		file = f + 1;
		rank = r + (-1) * opposit;
		valid_set(bb, file, rank);

		file = f + (-1);
		rank = r + 1 * opposit;
		valid_set(bb, file, rank);

		file = f + 1;
		rank = r + 1 * opposit;
		valid_set(bb, file, rank);

		return bb;
	}
	static bit::Bitboard init_gold_attacks(Side sd, File f, Rank r) {

		bit::Bitboard bb;
		bb.init();

		auto opposit = (sd == BLACK) ? 1 : -1;
		auto file = f + -1;
		auto rank = r + -1 * opposit;
		valid_set(bb, file, rank);

		file = f;
		rank = r + (-1) * opposit;
		valid_set(bb, file, rank);

		file = f + 1;
		rank = r + (-1) * opposit;
		valid_set(bb, file, rank);

		file = f + (-1);
		rank = r;
		valid_set(bb, file, rank);

		file = f + 1;
		rank = r;
		valid_set(bb, file, rank);

		file = f;
		rank = r + 1 * opposit;
		valid_set(bb, file, rank);

		return bb;
	}

	bit::Bitboard index_to_occ(const int index, const bit::Bitboard& bb) {
		bit::Bitboard ret_bb;
		bit::Bitboard mask = bb;
		ret_bb.init();
		auto bits = mask.pop_cnt();
		for (auto i = 0; i < bits; i++) {
			const auto sq = mask.lsb();
			if (index & (1 << i)) {
				ret_bb |= sq;
			}
		}
		return ret_bb;
	}
	bit::Bitboard calc_slider_att(const Square sq, const bit::Bitboard& occ, const bool is_diag1) {
		bit::Bitboard ret_bb;
		ret_bb.init();
		// diag1   diag2
		int file_dir_list[2][2] = { { 1, -1}, {-1, 1} };
		int rank_dir_list[2][2] = { {-1,  1}, {-1, 1} };
		for (auto i = 0; i < 2; i++) {
			const auto dir_file = (is_diag1) ? file_dir_list[0][i] : file_dir_list[1][i];
			const auto dir_rank = (is_diag1) ? rank_dir_list[0][i] : rank_dir_list[1][i];
			const auto file = square_file(sq);
			const auto rank = square_rank(sq);
			auto f = file + dir_file;
			auto r = rank + dir_rank;
			for (; is_valid(f, r); f += dir_file, r += dir_rank) {
				auto to_sq = square_make(f, r);
				ret_bb |= to_sq;
				if (occ.is_set(to_sq)) {
					break;
				}
			}
		}
		return ret_bb;
	}

	void init() {
		//mask init
		SQUARE_FOREACH(i) {
			g_mask[i].init();
			(!bit::Bitboard::select(i)) ?
				g_mask[i].select_set(0, (1ull << i)) :
				g_mask[i].select_set(1, (1ull << (i - 63)));
		}
		//all_one
		g_all_one_bb.init();
		g_all_zero_bb.init();
		SQUARE_FOREACH(sq) {
			g_all_one_bb.set(sq);
		}
		//rank mask init
		//file_mask init
		for (auto& bb : g_file_mask) {
			bb.init();
		}
		for (auto& bb : g_rank_mask) {
			bb.init();
		}
		SQUARE_FOREACH(i) {
			const auto file = square_file(i);
			const auto rank = square_rank(i);
			g_file_mask[file] |= i;
			g_rank_mask[rank] |= i;
		}
		//rook mask bishop mask int
		auto init_func = [](std::array<bit::Bitboard, SQUARE_SIZE>& bb_list,
			std::array<int, 2>file_inc,
			std::array<int, 2>rank_inc,
			bool is_bishop) {
				SQUARE_FOREACH(sq) {
					auto file = square_file(sq);
					auto rank = square_rank(sq);
					for (auto i = 0; i < 2; i++) {
						auto f_inc = file_inc[i];
						auto r_inc = rank_inc[i];
						auto f = file;
						auto r = rank;
						if (!f_inc && !r_inc) { continue; }
						while (true) {
							f += f_inc;
							r += r_inc;
							if (is_valid(f, r)) {
								bb_list[sq] |= square_make(f, r);
							}
							else {
								break;
							}
						}
					}
					if (is_bishop) {
						bb_list[sq] &= ~g_file_mask[File_1];
						bb_list[sq] &= ~g_file_mask[File_9];
						bb_list[sq] &= ~g_rank_mask[Rank_1];
						bb_list[sq] &= ~g_rank_mask[Rank_9];
					}
				}
		};
		SQUARE_FOREACH(sq) {
			g_diag1_mask[sq].init();
			g_diag2_mask[sq].init();
			g_lance_mask[BLACK][sq].init();
			g_lance_mask[WHITE][sq].init();
		}
		init_func(g_lance_mask[BLACK], { 0, 0 }, { -1, 0 }, false);
		init_func(g_lance_mask[WHITE], { 0, 0 }, { 0, 1 }, false);
		init_func(g_diag1_mask, { -1, +1 }, { +1, -1 }, true);
		init_func(g_diag2_mask, { -1, +1 }, { -1, +1 }, true);


		//prom middle
		g_prom[BLACK].init();
		g_prom[WHITE].init();
		g_middle[BLACK].init();
		g_middle[WHITE].init();
		SQUARE_FOREACH(sq) {
			const auto rank = square_rank(sq);
			if (rank == Rank_1 || rank == Rank_2 || rank == Rank_3) {
				g_prom[BLACK].set(sq);
			}
			if (rank == Rank_7 || rank == Rank_8 || rank == Rank_9) {
				g_prom[WHITE].set(sq);
			}
			if (rank == Rank_4 || rank == Rank_5
				|| rank == Rank_6 || rank == Rank_7
				|| rank == Rank_8 || rank == Rank_9) {
				g_middle[BLACK].set(sq);
			}
			if (rank == Rank_1 || rank == Rank_2
				|| rank == Rank_3 || rank == Rank_4
				|| rank == Rank_5 || rank == Rank_6) {
				g_middle[WHITE].set(sq);
			}
		}
		//double pawn mask
		for (auto index = 0; index < (1 << FILE_SIZE); index++) {
			g_double_pawn_mask[BLACK][index].init();
			g_double_pawn_mask[WHITE][index].init();
			for (auto shift = 0; shift < FILE_SIZE; shift++) {
				if (!(index & (1 << shift))) {
					g_double_pawn_mask[BLACK][index] |= g_file_mask[shift];
					g_double_pawn_mask[WHITE][index] |= g_file_mask[shift];
				}
			}
			g_double_pawn_mask[BLACK][index] &= ~g_rank_mask[Rank_1];
			g_double_pawn_mask[WHITE][index] &= ~g_rank_mask[Rank_9];
		}
		//init attack
		//attack table
		SQUARE_FOREACH(sq) {
			SIDE_FOREACH(sd) {
				auto file = square_file(sq);
				auto rank = square_rank(sq);
				g_knight_attacks[sd][sq] = init_knight_attacks(sd, file, rank);
				g_silver_attacks[sd][sq] = init_silver_attacks(sd, file, rank);
				g_gold_attacks[sd][sq] = init_gold_attacks(sd, file, rank);
			}
		}
		//init file attack table
		RANK_FOREACH(rank) {
			for (auto index = 0u; index < 128; index++) {
				auto inc_r = -1;
				bit::Bitboard bb = bit::Bitboard();
				bb.init();
				for (auto r = rank + inc_r; is_valid_rank(r); r += inc_r) {
					auto sq = square_make(File_1, r);
					bb |= sq;
					if ((index << 1) & (1 << r)) {
						break;
					}
				}
				inc_r = 1;
				for (auto r = rank + inc_r; is_valid_rank(r); r += inc_r) {
					auto sq = square_make(File_1, r);
					bb |= sq;
					if ((index << 1) & (1 << r)) {
						break;
					}
				}
				g_file_attack[rank][index] = bb.p<0>();
			}
		}
		//init rank attack table
		FILE_FOREACH(file) {
			for (auto index = 0u; index < 128; index++) {
				auto inc_f = -1;
				bit::Bitboard bb = bit::Bitboard();
				bb.init();
				for (auto f = file + inc_f; is_valid_file(f); f += inc_f) {
					auto sq = square_make(f, Rank_1);
					bb |= sq;
					if ((index << 1) & (1 << f)) {
						break;
					}
				}
				inc_f = 1;
				for (auto f = file + inc_f; is_valid_file(f); f += inc_f) {
					auto sq = square_make(f, Rank_1);
					bb |= sq;
					if ((index << 1) & (1 << f)) {
						break;
					}
				}
				g_rank_attack[file][index] = bb;
			}
		}
		auto diag1_offset = 0;
		auto diag2_offset = 0;
		SQUARE_FOREACH(sq) {
			auto max_index = 1ull << g_diag1_mask[sq].pop_cnt();
			g_diag1_offset[sq] = diag1_offset;
			for (auto index = 0ull; index < max_index; index++) {
				auto occ = index_to_occ(index, g_diag1_mask[sq]);
				g_diag1_attack[g_diag1_offset[sq] + occ_to_index(occ, g_diag1_mask[sq])] = calc_slider_att(sq, occ, true);
			}
			diag1_offset += max_index;
			max_index = 1ull << g_diag2_mask[sq].pop_cnt();
			g_diag2_offset[sq] = diag2_offset;
			for (auto index = 0ull; index < max_index; index++) {
				auto occ = index_to_occ(index, g_diag2_mask[sq]);
				g_diag2_attack[g_diag2_offset[sq] + occ_to_index(occ, g_diag2_mask[sq])] = calc_slider_att(sq, occ, false);

			}
			diag2_offset += max_index;
		}
		//init between table
		SQUARE_FOREACH(sq1) {
			SQUARE_FOREACH(sq2) {
				gBetweenIndex[sq1][sq2] = gBeyondIndex[sq1][sq2] = 0;
			}
		}
		auto between_sq = 0;
		auto beyond_sq = 0;
		SQUARE_FOREACH(from) {
			SQUARE_FOREACH(to) {
				auto from_f = square_file(from);
				auto from_r = square_rank(from);
				auto to_f = square_file(to);
				auto to_r = square_rank(to);
				Bitboard between_bb;
				Bitboard beyond_bb;
				between_bb.init();
				beyond_bb.init();
				gBetweenIndex[from][to] = 0;
				gBeyondIndex[from][to] = 0;
				int inc[8][2] = { {1,0,},{-1,0},{0,-1},{0,1},{1,1},{-1,-1},{1,-1},{-1,1} };
				for (auto inc1 : inc) {
					File f;
					Rank r;
					for (f = from_f + inc1[0], r = from_r + inc1[1]; is_valid(f, r); f += inc1[0], r += inc1[1]) {
						if (f == to_f && r == to_r) {
							//between
							for (f = from_f + inc1[0], r = from_r + inc1[1];; f += inc1[0], r += inc1[1]) {
								if (f == to_f && r == to_r) { break; }
								auto sq = square_make(File(f), Rank(r));
								between_bb.set(sq);
							}
							//beyond
							for (f = to_f + inc1[0], r = to_r + inc1[1]; is_valid(f, r); f += inc1[0], r += inc1[1]) {
								auto sq = square_make(File(f), Rank(r));
								beyond_bb.set(sq);
							}
							goto loop_end;
						}
					}
				}
			loop_end:

				auto found_flag = false;
				for (auto i = 0; i < between_sq; i++) {
					if (between_bb == gBetween[i]) {
						found_flag = true;
						gBetweenIndex[from][to] = i;
						break;
					}
				}
				if (!found_flag) {
					gBetween[between_sq] = between_bb;
					gBetweenIndex[from][to] = between_sq++;
				}
				found_flag = false;
				for (auto i = 0; i < beyond_sq; i++) {
					if (beyond_bb == gBeyond[i]) {
						found_flag = true;
						gBeyondIndex[from][to] = i;
						break;
					}
				}
				if (!found_flag) {
					gBeyond[beyond_sq] = beyond_bb;
					gBeyondIndex[from][to] = beyond_sq++;
				}
			}
		}
		//Tee << "between:" << between_sq << std::endl;
		//Tee << "beyond:" << beyond_sq << std::endl;

		//init check area
		SIDE_FOREACH(sd) {
			PIECE_FOREACH(pc) {
				SQUARE_FOREACH(king_sq) {
					auto result_bb = bit::g_all_zero_bb;
					auto prom_pc = piece_can_prom(pc) ? piece_prom(pc) : pc;
					auto opp = flip_turn(sd);
					auto occ = bit::g_all_zero_bb;
					//noprom -> noprom
					auto attack_bb = piece_attacks(pc,opp,king_sq,occ);
					
					while(attack_bb) {
						auto to_sq = attack_bb.lsb();
						auto attack_bb2 = piece_attacks(pc,opp,to_sq,occ);
						while(attack_bb2) {
							auto from_sq = attack_bb2.lsb();
							result_bb.set(from_sq);
						}
					}

					//noprom -> prom
					attack_bb = piece_attacks(prom_pc,opp,king_sq,occ);
					while(attack_bb) {
						auto to_sq = attack_bb.lsb();
						auto attack_bb2 = piece_attacks(pc,opp,to_sq,occ);
						auto to_can_prom = square_is_prom(sd,to_sq);
						while(attack_bb2) {
							auto from_sq = attack_bb2.lsb();
							auto from_can_prom = square_is_prom(sd,from_sq);
							if(from_can_prom || to_can_prom) {
								result_bb.set(from_sq);
							}
						}
					}

					g_check_area[sd][pc][king_sq] = result_bb;
					//Tee<<"sd:"<<sd<<"pc:"<<pc<<"king_sq:"<<king_sq<<std::endl;
					//Tee<<result_bb<<std::endl;

				}
			}
		}

	}

	Bitboard piece_attacks(const Piece pc, const Side sd, const Square from, const Bitboard& pieces) {
		switch (pc) {
		case Pawn: return get_pawn_attack(sd, from);
		case Knight: return get_knight_attack(sd, from);
		case Silver: return get_silver_attack(sd, from);
		case Gold:
		case Golds:
		case PPawn:
		case PLance:
		case PKnight:
		case PSilver: return get_gold_attack(sd, from);
		case King: return get_king_attack(from);
		case Rook: return get_rook_attack(from, pieces);
		case Bishop: return get_bishop_attack(from, pieces);
		case PRook: return get_prook_attack(from, pieces);
		case PBishop: return get_pbishop_attack(from, pieces);
		case Lance: return get_lance_attack(sd, from, pieces);
		default: return Bitboard(0ull, 0ull);
		}
	}

	void test() {
		std::cout << "test bit start\n";
		{
			auto bb = bit::Bitboard();
			bb.init();
			Tee << bb << std::endl;
		}
		std::cout << "test bit end\n";

	}

}