#ifndef HASH_HPP
#define HASH_HPP

#include "common.hpp"

class Pos;

namespace hash {
extern Key gKeyTurn;
extern Key gKeyPiece[SIDE_SIZE][PIECE_SIZE][SQUARE_SIZE];
extern Key gKeyHand[SIDE_SIZE][PIECE_SIZE][20];

void init();
Key key(const Pos &pos);
Key key_piece(const Pos &pos);
inline Key key_turn() {
    return gKeyTurn;
}
inline Key key_turn(Side sd) {
    return (sd == BLACK) ? Key(0) : gKeyTurn;
}
inline Key key_piece(const Piece pc, const Side sd, const Square sq) {
    return gKeyPiece[sd][pc][sq];
}
inline Key key_hand(const Piece pc, const Side sd, const int num) {
    return gKeyHand[sd][pc][num];
}
inline int index(Key key, int mask) {
    return int(key) & mask;
}
inline uint32 lock(Key key) {
    return uint32(uint64(key) >> 32);
}
}


#endif