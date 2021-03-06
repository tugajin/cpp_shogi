#include "attack.hpp"
#include "pos.hpp"
#include "gen.hpp"
#include "list.hpp"
#include "sfen.hpp"
#include "eval.hpp"

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
template<bool skip_king, bool skip_pawn>bit::Bitboard attacks_to(const Pos& pos, const Side sd, const Square sq, const bit::Bitboard& pieces) {
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
bit::Bitboard attack_from(const Side sd, const Square sq, const Piece pc, const bit::Bitboard& pieces) {
	switch(pc) {
		case Pawn:
			return get_pawn_attack(sd, sq);
		case Lance:
			return get_lance_attack(sd, sq, pieces);
		case Knight:
			return get_knight_attack(sd, sq);
		case Silver:
			return get_silver_attack(sd, sq);
		case Gold:
		case PPawn:
		case PLance:
		case PKnight:
		case PSilver:
			return get_gold_attack(sd, sq);
		case King:
			return get_king_attack(sq);
		case Rook:
			return get_rook_attack(sq, pieces);
		case Bishop:
			return get_bishop_attack(sq, pieces);
		case PRook:
			return get_prook_attack(sq, pieces);
		case PBishop:
			return get_pbishop_attack(sq, pieces);
		default:
			assert(false);
			return bit::Bitboard(0ull, 0ull);
	}
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
bool has_attack(const Pos& pos, const Side sd, const Square sq, const bit::Bitboard& pieces) {
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
bit::Bitboard pins(const Pos& pos, const Side sd) {
	auto pins = g_all_zero_bb;
	const auto xd = flip_turn(sd);
	const auto king = pos.king(sd);
	auto attacks = get_pseudo_bishop_attack(king);
	auto b = (pos.pieces(Bishop) | pos.pieces(PBishop)) & pos.pieces(xd) & attacks;
	while (b) {
		const auto ds = b.lsb();
		const auto btwn_bb = between(king,ds) & pos.pieces();
		if (btwn_bb.pop_cnt() == 1) {
			pins |= btwn_bb;
		}
	}
	attacks = get_pseudo_rook_attack(king);
	b = (pos.pieces(Rook) | pos.pieces(PRook)) & pos.pieces(xd) & attacks;
	while (b) {
		const auto ds = b.lsb();
		const auto btwn_bb = between(king,ds) & pos.pieces();
		if (btwn_bb.pop_cnt() == 1) {
			pins |= btwn_bb;
		}
	}
	//縦のattacksが残っている
	b = pos.pieces(Lance, xd) & g_lance_mask[sd][king] & attacks;
	while (b) {
		const auto ds = b.lsb();
		const auto btwn_bb = between(king,ds) & pos.pieces();
		if (btwn_bb.pop_cnt() == 1) {
			pins |= btwn_bb;
		}
	}
	return pins;
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

static Square pick_lva(const Pos & pos, const Side sd, const Square to, const Bitboard &pieces) {

	PIECE_FOREACH(pc){

		auto froms = pos.pieces(pc, sd) & pieces & bit::piece_attacks(pc, sd, to, pieces);

		for (auto b = froms; b;) {
			Square from = b.lsb();
			if (bit::line_is_empty(from, to, pieces)) return from;
		}
	}
	return SQ_NONE;
}



static Score see_rec(const Pos & pos, const Side sd, const Square to, const Bitboard & pieces, const Piece cp) {

	assert(cp != PieceNone);

	Score bs = Score(0); // stand pat

	Square from = pick_lva(pos, sd, to, pieces);

	if (from != SQ_NONE) {

		Piece pc = pos.piece(from);

		Score sc = piece_material_ex(cp);
		auto new_pieces = pieces;
		new_pieces.clear(from);
		if (cp != King) sc -= see_rec(pos, flip_turn(sd), to, new_pieces, pc);

		if (sc > bs) bs = sc;
	}

	assert(bs >= 0);
	return bs;
}

Score see_max(Move mv) {

	Score sc = Score(0);
	const auto cap = move::move_cap(mv);
	if (move::move_is_cap(mv)) sc += piece_material_ex(cap);
	const auto piece = move::move_piece(mv);
	if (move::move_is_prom(mv)) sc += piece_material_pm(piece);

	return sc;
}

Score see(const Move mv, const Pos & pos) {

	assert(pos.is_ok());
	assert(move::move_is_ok(mv,pos));

	Square from = move::move_from(mv);
	Square to   = move::move_to(mv);

	Piece pc = move::move_piece(mv);
	Side  sd = pos.turn();

	Score sc = Score(0);
	const auto cap = move::move_cap(mv);
	if (move::move_is_cap(mv)) sc += piece_material_ex(cap);

	if (move::move_is_prom(mv)) {
		sc += piece_material_pm(pc);
		pc = piece_prom(pc);
	}
	auto pieces = pos.pieces();
	if (!move::move_is_drop(mv)) {
		pieces.clear(from);
	}

	sc -= see_rec(pos, flip_turn(sd), to, pieces, pc);

	return sc;
}

bit::Bitboard discover_attacks_rec(const Square from, const Pos & pos, const bit::Bitboard & direct_att, const bit::Bitboard & pseudo_att) {

	const auto pc = pos.piece(from);
	const auto sd = pos.turn();

	if(!is_slider(pc)) {
		return (attack_from(sd, from, pc, pos.pieces()) & (~direct_att) & pseudo_att);
	}
	const auto occ = pos.pieces();
	bit::Bitboard direct_attacks;
	switch(pc) {
		case Lance:
			direct_attacks = get_lance_attack(sd, from, occ);
			break;
		case Rook:
		case PRook:
			direct_attacks = get_rook_attack(from, occ);
			break;
		case Bishop:
		case PBishop:
			direct_attacks = get_bishop_attack(from, occ);
			break;
		default:
			assert(false);
			break;
	}
	
	direct_attacks.set(from);

	auto last_att = direct_attacks & (~direct_att) & pseudo_att;
	auto next_bb = last_att & pos.pieces(sd);
	if(!next_bb) {
		return last_att;
	}
	const auto next = next_bb.lsb();
	return discover_attacks_rec(next,pos,direct_attacks,pseudo_att);
}

bit::Bitboard discover_attacks(const Square from, const Pos & pos) {

	auto attacks = g_all_zero_bb;

	const auto pc = pos.piece(from);
	if(!is_slider(pc)) {
		return attacks;
	}

	const auto sd = pos.turn();
	const auto occ = pos.pieces();

	bit::Bitboard pseudo_attacks, direct_attacks;
	switch(pc) {
		case Lance:
			pseudo_attacks = get_pseudo_lance_attack(sd, from);
			direct_attacks = get_lance_attack(sd, from, occ);
			break;
		case Rook:
		case PRook:
			pseudo_attacks = get_pseudo_rook_attack(from);
			direct_attacks = get_rook_attack(from, occ);
			break;
		case Bishop:
		case PBishop:
			pseudo_attacks = get_pseudo_bishop_attack(from);
			direct_attacks = get_bishop_attack(from, occ);
			break;
		default:
			assert(false);
			return attacks;
	}

	direct_attacks.set(from);

	for(auto discover_bb = direct_attacks & pos.pieces(sd); discover_bb;) {
		const auto next = discover_bb.lsb();
		attacks |= discover_attacks_rec(next,pos,direct_attacks,pseudo_attacks);
	}

	return attacks;
}

namespace attack {
	void test() {
		{
			Pos pos = pos_from_sfen("8k/4b4/9/4P4/9/9/4L4/1S2RR1G1/8K b");
			Tee << pos << std::endl;
			Tee<< discover_attacks(SQ_58, pos)<<std::endl;
		}
		/*{
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
		}*/
	}
}

template bit::Bitboard attacks_to<true, true>(const Pos& pos, const Side sd, const Square sq, const bit::Bitboard & pieces);
template bit::Bitboard attacks_to<true, false>(const Pos& pos, const Side sd, const Square sq, const bit::Bitboard & pieces);
template bit::Bitboard attacks_to<false, true>(const Pos& pos, const Side sd, const Square sq, const bit::Bitboard & pieces);
template bit::Bitboard attacks_to<false, false>(const Pos& pos, const Side sd, const Square sq, const bit::Bitboard & pieces);
