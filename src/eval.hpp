#ifndef EVAL_HPP
#define EVAL_HPP
#include "common.hpp"
#include <cmath>

class Pos;
template<Side sd> Score eval(const Pos& pos);
template<Side sd> Score material(const Pos& pos);
Score eval(const Pos& pos);
Score material(const Pos& pos);
Score piece_material(const Piece pc);
Score piece_material_ex(const Piece pc);
Score piece_material_pm(const Piece pc);
template<Side sd> UCTScore uct_eval(const Pos& pos);
inline double sigmoid(double x) {
	return 1 / (1 + std::exp(-x / 2000));
}
inline double sigmoid_inverse(double x) {
	return std::log(x / (1 - x)) * 2000;
}


#endif
