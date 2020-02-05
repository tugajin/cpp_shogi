#ifndef MOVE_HPP
#define MOVE_HPP

#include <string>
#include "libmy.hpp"
#include "common.hpp"

class Pos;

namespace move {

Move from_usi(const std::string &s, const Pos &pos);

// move structure
//xxxxxxxx xxxxxxxx xxxxxxxx x1111111  destination
//xxxxxxxx xxxxxxxx xx111111 1xxxxxxx  starting square(drop move = square::SIZE)
//xxxxxxxx xxxxxxxx x1xxxxxx xxxxxxxx  flag for promotion
//xxxxxxxx xxxxx111 1xxxxxxx xxxxxxxx  piece to move
//xxxxxxxx x1111xxx xxxxxxxx xxxxxxxx  captured piece


// move16 structure
//xxxxxxxx xxxxxxxx xxxxxxxx x1111111  destination
//xxxxxxxx xxxxxxxx xx111111 1xxxxxxx  starting square(drop move = square::SIZE + piece)
//xxxxxxxx xxxxxxxx x1xxxxxx xxxxxxxx  flag for promotion

namespace move {

constexpr int TO_SHIFT = 0;
constexpr int FROM_SHIFT = 7;
constexpr int PROM_SHIFT = 14;
constexpr int PIECE_SHIFT = 15;
constexpr int CAP_SHIFT = 19;

constexpr int BITS = 24;
constexpr int SIZE = 1 << BITS;
constexpr int MASK = SIZE - 1;

constexpr uint32 MOVE16_MASK = 0x7FFF;

}
const Move MOVE_NONE = Move(0);
const Move MOVE_NULL = Move((1u<<(move::BITS+1))-1);
//move
inline Move make_move(const Square f, const Square t, const Piece pc, const Piece cp, const bool pp =false) {
    return Move((f << move::FROM_SHIFT) | (t << move::TO_SHIFT) | (pc << move::PIECE_SHIFT)
      | (cp << move::CAP_SHIFT) | (static_cast<int>(pp) << move::PROM_SHIFT));
}
//drop
inline Move make_drop_move(const int t, const int pc) {
    return Move((SQUARE_SIZE << move::FROM_SHIFT) | (t << move::TO_SHIFT) | (pc << move::PIECE_SHIFT));
}
inline Square move_from(const Move mv) {
    return Square((uint32(mv) >> move::FROM_SHIFT) & 0x7f);
}
inline Square move_to(const Move mv) {
    return Square((uint32(mv) >> move::TO_SHIFT) & 0x7f);
}
inline Piece move_piece(const Move mv) {
    return Piece((uint32(mv) >> move::PIECE_SHIFT) & 0xf);
}
inline Piece move_cap(const Move mv) {
    return Piece((uint32(mv) >> move::CAP_SHIFT) & 0xf);
}
inline bool move_is_prom(const Move mv) {
    return ((uint32(mv) & (1 << move::PROM_SHIFT)) != 0);
}
inline bool move_is_drop(const Move mv) {
    return move_from(mv) >= SQUARE_SIZE;
}
std::string move_to_usi(const Move mv);

}

#endif