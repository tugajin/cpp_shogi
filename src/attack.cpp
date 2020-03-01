#include "attack.hpp"
#include "pos.hpp"
#include "gen.hpp"
#include "list.hpp"
#include "sfen.hpp"

static bool can_play(const Pos& pos) {
	List list;
	gen_moves(list, pos);
	for (auto i = 0; i < list.size(); ++i) {
		const auto mv = list[i];
		if (move::pseudo_is_legal(mv, pos)) {
			return true;
		}
	}
	return false;
}
static bool in_check(const Pos& pos, const Side sd) {
	return has_attack(pos, flip_turn(sd), pos.king(sd));
}
bool is_legal(const Pos& pos) {
	return !in_check(pos, flip_turn(pos.turn()));
}
bool is_mate(const Pos& pos) {
	return !can_play(pos);
}
bool in_check(const Pos& pos) {
	return bool(checks(pos));
}
template<bool skip_king, bool skip_pawn>bit::Bitboard attacks_to(const Pos& pos, const Side sd, const Square sq, const bit::Bitboard pieces) {
	Bitboard bb(0ull, 0ull);
	Bitboard attack, piece;
	const auto xd = flip_turn(sd);
	if (!skip_pawn) {
		//pawn
		piece = pos.pieces(Pawn, sd);
		attack = get_pawn_attack(xd, sq);
		bb |= piece & attack;
	}
	//knight
	piece = pos.pieces(Knight, sd);
	attack = get_knight_attack(xd, sq);
	bb |= piece & attack;
	//silver
	piece = pos.pieces(Silver, sd);
	attack = get_silver_attack(xd, sq);
	bb |= piece & attack;
	//gold
	piece = pos.golds(sd);
	attack = get_gold_attack(xd, sq);
	bb |= piece & attack;
	//king
	if (!skip_king) {
		piece = (pos.pieces(King) | pos.pieces(PRook) | pos.pieces(PBishop)) & pos.pieces(sd);
	}
	else {
		piece = (pos.pieces(PRook) | pos.pieces(PBishop)) & pos.pieces(sd);
	}
	attack = get_king_attack(sq);
	bb |= piece & attack;
	//rook rank
	piece = (pos.pieces(Rook) | pos.pieces(PRook)) & pos.pieces(sd);
	attack = get_rank_attack(sq, pieces);
	bb |= piece & attack;
	//rook lance file
	piece |= pos.pieces(Lance, sd) & g_lance_mask[xd][sq];
	attack = get_file_attack(sq, pieces);
	bb |= piece & attack;
	//bishop
	piece = (pos.pieces(Bishop) | pos.pieces(PBishop)) & pos.pieces(sd);
	attack = get_bishop_attack(sq, pieces);
	bb |= piece & attack;

	return bb;
}
bit::Bitboard checks(const Pos& pos) {
	const auto xd = pos.turn();
	const auto sd = flip_turn(xd);
	const auto king = pos.king(xd);
	const auto pieces = pos.pieces();
	return attacks_to<false, false>(pos, sd, king, pieces);
}
bool move_is_safe(const Move /*mv*/, const Pos&/*pos*/) {
	//todo
	return true;
}
bool move_is_win(const Move /*mv*/, const Pos&/*pos*/) {
	//todo
	return true;
}

bool has_attack(const Pos& pos, const Side sd, const Square sq) {
	return has_attack(pos, sd, sq, pos.pieces());
}
bool has_attack(const Pos& pos, const Side sd, const Square sq, bit::Bitboard pieces) {
	Bitboard attack, piece;
	const auto xd = flip_turn(sd);
	//gold
	piece = pos.golds(sd);
	attack = get_gold_attack(xd, sq);
	if ((piece & attack)) { return true; }
	//silver
	piece = pos.pieces(Silver, sd);
	attack = get_silver_attack(xd, sq);
	if ((piece & attack)) { return true; }
	//king
	piece = (pos.pieces(King) | pos.pieces(PRook) | pos.pieces(PBishop)) & pos.pieces(sd);
	attack = get_king_attack(sq);
	if ((piece & attack)) { return true; }
	//rook rank
	piece = (pos.pieces(Rook) | pos.pieces(PRook)) & pos.pieces(sd);
	attack = get_rank_attack(sq, pieces);
	if ((piece & attack)) { return true; }
	//rook lance file
	piece |= pos.pieces(Lance, sd) & g_lance_mask[xd][sq];
	attack = get_file_attack(sq, pieces);
	if ((piece & attack)) { return true; }
	//bishop
	piece = (pos.pieces(Bishop) | pos.pieces(PBishop)) & pos.pieces(sd);
	attack = get_bishop_attack(sq, pieces);
	if ((piece & attack)) { return true; }

	//knight
	piece = pos.pieces(Knight, sd);
	attack = get_knight_attack(xd, sq);
	if ((piece & attack)) { return true; }

	//pawn
	piece = pos.pieces(Pawn, sd);
	attack = get_pawn_attack(xd, sq);
	if ((piece & attack)) { return true; }

	return false;
}

bool is_pinned(const Pos&/*pos*/, const Square /*king*/, const Square /*sq*/, const Side /*sd*/) {
	return false;
}
Square pinned_by(const Pos&/*pos*/, const Square /*king*/, const Square /*sq*/, const Side /*sd*/) {

	return SQ_11;
}
bit::Bitboard pins(const Pos&/*pos*/, const Square /*king*/) {
	return Bitboard(0, 0);
}

bool is_mate_with_pawn_drop(const Square to, const Pos& pos) {
	const auto sd = pos.turn();
	const auto xd = flip_turn(pos.turn());
	const auto king_sq = pos.king(xd);

#ifdef DEBUG
	if (sd == BLACK) {
		if (to + Inc_N != king_sq) {
			Tee << "mate with pawn drop error\n";
			Tee << pos << std::endl;
			Tee << to << std::endl;
			Tee << king_sq << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	else {
		if (to + Inc_S != king_sq) {
			Tee << "mate with pawn drop error\n";
			Tee << pos << std::endl;
			Tee << to << std::endl;
			Tee << king_sq << std::endl;
			exit(EXIT_FAILURE);
		}
	}
#endif
	//capture checher
	auto pieces = pos.pieces();
	assert(!pieces.is_set(to));
	pieces.set(to);
	auto xd_piece = attacks_to<true, true>(pos, xd, to, pieces);
	while (xd_piece) {
		const auto from = xd_piece.lsb();
		pieces.clear(from);
		if (!has_attack(pos, sd, king_sq, pieces)) {
			return false;
		}
		pieces.set(from);
	}
	//escape king
	auto dist = get_king_attack(king_sq) & (~pos.pieces(xd));
	//pieces.clear(king_sq);//いらないかも
	while (dist) {
		const auto king_to = dist.lsb();
		if (!has_attack(pos, sd, king_to, pieces)) {
			return false;
		}
	}
	return true;
}

namespace attack {
	void test() {
		{
			Pos pos = pos_from_sfen("3gksR2/9/4+P4/9/9/9/9/9/4K4 b P");
			Tee << pos << std::endl;
			List list;
			auto bb = Bitboard(0ull, 0ull);
			gen_moves<DROP, BLACK>(list, pos, &bb);
			Tee << list << std::endl;
		}
		{
			Pos pos = pos_from_sfen("R2gksR2/9/4+P4/9/9/9/9/9/4K4 b P");
			Tee << pos << std::endl;
			List list;
			auto bb = Bitboard(0ull, 0ull);
			gen_moves<DROP, BLACK>(list, pos, &bb);
			Tee << list << std::endl;
		}
		{
			Pos pos = pos_from_sfen("l6nl/5+P2k/3p1S1g1/p1p5p/3n2SpP/1PPb2P2/P5GS1/R5K2/LN3sb1L w RG7png");
			Tee << is_legal(pos) << std::endl;
		}
		{
			Pos pos = pos_from_sfen("lnsgkgsnl/1r7/ppppppppb/8p/9/4P3P/PPPP1PPP1/1B5R1/LNSGKGSNL b");
			Tee << pos << std::endl;
			List list;
			gen_legals(list, pos);
			Tee << list << std::endl;
		}
		{
			Pos pos = pos_from_sfen("lnsgkgsnl/1r7/pppppp1pp/6p2/2b6/8P/PPPPSPPP1/1B3K1R1/LNSG1GSNL b p");
			Tee << pos << std::endl;
			List list;
			gen_legals(list, pos);
			Tee << list << std::endl;
		}
	}
}

template bit::Bitboard attacks_to<true, true>(const Pos& pos, const Side sd, const Square sq, const bit::Bitboard pieces);
template bit::Bitboard attacks_to<true, false>(const Pos& pos, const Side sd, const Square sq, const bit::Bitboard pieces);
template bit::Bitboard attacks_to<false, true>(const Pos& pos, const Side sd, const Square sq, const bit::Bitboard pieces);
template bit::Bitboard attacks_to<false, false>(const Pos& pos, const Side sd, const Square sq, const bit::Bitboard pieces);
