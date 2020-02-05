#ifndef GEN_HPP
#define GEN_HPP

#include "common.hpp"
#include "libmy.hpp"

class List;
class Pos;
namespace bit {
class Bitboard;
}

void gen_legals(List &list, const Pos &pos);
void gen_moves(List &list, const Pos &pos);
void gen_moves(List &list, const Pos &pos, const bit::Bitboard &checks);
void gen_evasions(List &list, const Pos &pos, const bit::Bitboard &checks);
void gen_captures(List &list, const Pos &pos);


#endif