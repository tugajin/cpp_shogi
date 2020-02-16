#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "common.hpp"
#include "libmy.hpp"
#include "pos.hpp"
#include "score.hpp"
#include "util.hpp"

class List;
class Pos;

const Depth DEPTH_MAX  = Depth(128);
const Ply PLY_ROOT = Ply(0);
const Ply PLY_MAX = Ply(127);
const int PLY_SIZE = PLY_MAX + 1;
constexpr int MAX_LEGAL_MOVES = 600;

class Line {
private:
    ml::Array<Move, PLY_SIZE> move_;
public:
    Line();
    void clear();
    void add(const Move mv);
};
class SearchInput {
    public:
    void init();
};

namespace search {
    void test();
}
#endif