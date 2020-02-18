#ifndef EVAL_HPP
#define EVAL_HPP
#include "common.hpp"
class Pos;
template<Side sd> Score eval(const Pos &pos);
template<Side sd> Score material(const Pos &pos);
Score eval(const Pos &pos);
Score material(const Pos &pos);
template<Side sd> UCTScore uct_eval(const Pos &pos);


#endif