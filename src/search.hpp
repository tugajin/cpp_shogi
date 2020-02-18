#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "common.hpp"
#include "libmy.hpp"
#include "pos.hpp"
#include "score.hpp"
#include "util.hpp"
#include "move.hpp"

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
    Line() {
        this->clear();
    }
    void clear() {
        move_.clear();
    }
    void add(const Move mv) {
        assert(move::move_is_ok(mv));
        move_.add(mv);
    }
    void set(const Move mv) {
        clear();
        add(mv);
    }
    void concat(const Move mv, const Line &pv) {
        clear();
        add(mv);
        for(auto i = 0; i < pv.size(); ++i) {
            add(pv[i]);
        }
    }
    int size() const {
        return move_.size();
    }
    Move move(const int i) const {
        return move_[i];
    }
    Move operator[](const int i) const {
        return move(i);
    }
    std::string to_usi() const {
        std::string s = "";
        for(auto i = 0; i < this->size(); i++) {
            if(!s.empty()) { s+= " "; }
            s +=  move::move_to_usi(move(i)) ;
        }
        return s;
    }
};
class SearchInput {
    public:
    void init();
};

namespace search {
    void test();
}
#endif