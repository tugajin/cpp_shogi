
#ifndef ATTACK_HPP
#define ATTACK_HPP

#include "common.hpp"
#include "libmy.hpp"
#include "bit.hpp"

class Pos;

bool is_mate(const Pos& pos);
bool is_legal(const Pos& pos);
bool in_check(const Pos& pos);
bit::Bitboard checks(const Pos& pos);
bool move_is_safe(const Move mv, const Pos& pos);
bool move_is_win(const Move mv, const Pos& pos);

bool has_attack(const Pos& pos, const Side sd, const Square sq);
bool has_attack(const Pos& pos, const Side sd, const Square sq, bit::Bitboard pieces);

template<bool skip_king, bool skip_pawn>bit::Bitboard attacks_to(const Pos& pos, const Side sd, const Square sq, const bit::Bitboard pieces);

bool is_pinned(const Pos& pos, const Square king, const Square sq, const Side sd);
Square pinned_by(const Pos& pos, const Square king, const Square sq, const Side sd);
bit::Bitboard pins(const Pos& pos, const Side sd);

bool is_mate_with_pawn_drop(const Square to, const Pos& pos);

Score see(const Move mv, const Pos & pos);

namespace attack {
	void test();
}

#endif
