#include "eval.hpp"
#include "pos.hpp"
#include <cmath>

constexpr Score piece_value [] = 
    { Score(0),  Score(100), Score(300), Score(300), Score(400), Score(700),Score(800), Score(500), Score(15000),
      Score(510),Score(500), Score(500), Score(500), Score(500), Score(850),Score(900),
    };
double sigmoid(double x) {
    return 1/(1+std::exp(-x/2000));
}
template<Side sd> UCTScore uct_eval(const Pos &pos) {
    auto score = eval<sd>(pos);
    auto uct_score = sigmoid(double(score));
    return uct_score;
}

template<Side sd> Score eval(const Pos &pos) {
    return material<sd>(pos);
}
template<Side sd> Score material(const Pos &pos) {
    auto score = Score(0);
    auto bb = pos.pieces(BLACK);
    while(bb) {
        const auto sq = bb.lsb();
        const auto pc = pos.piece(sq);
        score += piece_value[pc];
    }
    bb = pos.pieces(WHITE);
    while(bb) {
        const auto sq = bb.lsb();
        const auto pc = pos.piece(sq);
        score -= piece_value[pc];
    }
    HAND_FOREACH(pc) {
        score += piece_value[pc] * hand_num(pos.hand(BLACK),pc);
        score -= piece_value[pc] * hand_num(pos.hand(WHITE),pc);
    }
    return (sd == BLACK) ? score : -score;
}
Score eval(const Pos &pos) {
    return pos.turn() == BLACK ? eval<BLACK>(pos) : eval<WHITE>(pos);
}
Score material(const Pos &pos) {
    return pos.turn() == BLACK ? material<BLACK>(pos) : material<WHITE>(pos);
}