#ifndef GEN_HPP
#define GEN_HPP

#include "common.hpp"
#include "libmy.hpp"

class List;
class Pos;
namespace bit {
class Bitboard;
}

enum MoveType {
    TACTICAL, QUIET, DROP, EVASION,
};

template<Side sd>void gen_legals(List &list, const Pos &pos);
template<Side sd>void gen_moves(List &list, const Pos &pos);
template<Side sd>void gen_moves(List &list, const Pos &pos, const bit::Bitboard *checks);

void gen_legals(List &list, const Pos &pos);
void gen_moves(List &list, const Pos &pos);
template<MoveType mt>void gen_moves(List &list, const Pos &pos);
template<MoveType mt>void gen_moves(List &list, const Pos &pos, const bit::Bitboard *checks);
template<MoveType mt, Side sd>void gen_moves(List & ml, const Pos & pos, const bit::Bitboard *checks);

namespace gen {
    void test();
}

#endif