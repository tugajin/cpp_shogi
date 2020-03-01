#include "eval.hpp"
#include "pos.hpp"
#include <cmath>
#include <random>
//#include <torch/torch.h>

constexpr Score piece_value[] =
{ Score(0),  Score(100), Score(300), Score(300), Score(400), Score(700), Score(800), Score(500), Score(15000),
			 Score(510) ,Score(500), Score(500), Score(500), Score(850),Score(900),
};
static_assert(piece_value[Pawn] == Score(100), "pawn value error");
static_assert(piece_value[Lance] == Score(300), "lance value error");
static_assert(piece_value[Knight] == Score(300), "knight value error");
static_assert(piece_value[Silver] == Score(400), "silver value error");
static_assert(piece_value[Gold] == Score(500), "gold value error");
static_assert(piece_value[Bishop] == Score(700), "bishop value error");
static_assert(piece_value[Rook] == Score(800), "rook value error");
static_assert(piece_value[King] == Score(15000), "king value error");
static_assert(piece_value[PPawn] == Score(510), "ppawn value error");
static_assert(piece_value[PLance] == Score(500), "plance value error");
static_assert(piece_value[PKnight] == Score(500), "pknight value error");
static_assert(piece_value[PSilver] == Score(500), "psilver value error");
static_assert(piece_value[PBishop] == Score(850), "pbishop value error");
static_assert(piece_value[PRook] == Score(900), "prook value error");


template<Side sd> UCTScore uct_eval(const Pos& pos) {
	auto score = eval<sd>(pos);
	std::random_device rd;
	score += Score(rd() % 30);
	auto uct_score = sigmoid(double(score));
	// torch::Tensor tensor = torch::rand({ 2, 3 });
	// std::cout << tensor << std::endl;
	return uct_score;
}

template<Side sd> Score eval(const Pos& pos) {
	return material<sd>(pos);
}
template<Side sd> Score material(const Pos& pos) {
	auto score = Score(0);
	auto bb = pos.pieces(BLACK);
	while (bb) {
		const auto sq = bb.lsb();
		const auto pc = pos.piece(sq);
		score += piece_value[pc];
	}
	bb = pos.pieces(WHITE);
	while (bb) {
		const auto sq = bb.lsb();
		const auto pc = pos.piece(sq);
		score -= piece_value[pc];
	}
	HAND_FOREACH(pc) {
		score += piece_value[pc] * hand_num(pos.hand(BLACK), pc);
		score -= piece_value[pc] * hand_num(pos.hand(WHITE), pc);
	}
	return (sd == BLACK) ? score : -score;
}
Score eval(const Pos& pos) {
	return pos.turn() == BLACK ? eval<BLACK>(pos) : eval<WHITE>(pos);
}
Score material(const Pos& pos) {
	return pos.turn() == BLACK ? material<BLACK>(pos) : material<WHITE>(pos);
}
UCTScore uct_eval(const Pos& pos) {
	return pos.turn() == BLACK ? uct_eval<BLACK>(pos) : uct_eval<WHITE>(pos);
}