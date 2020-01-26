#ifndef HAND_HPP
#define HAND_HPP

#include "common.hpp"

//handの構成
//     rook      bishop      gold       silver     knight      lance      pawn
//1bit 2bit 1bit 2bit 1bit   3bit 1bit  3bit 1bit  3bit  1bit  3bit 1bit  5bit
                                 //g  p  l  n   s   b   r
constexpr uint32 hand_shift[] = { 0, 6, 10, 14, 22, 25, 18 };

constexpr uint32 hand_mask[] = { 0x1fu << hand_shift[0], 0x7u << hand_shift[1],
                                 0x7u << hand_shift[2], 0x7u << hand_shift[3], 0x3u << hand_shift[4],
                                 0x3u << hand_shift[5], 0x7u << hand_shift[6] };

constexpr uint32 hand_inc[] = { 1u << hand_shift[0], 1u << hand_shift[1], 
                                1u << hand_shift[2], 1u << hand_shift[3], 
                                1u << hand_shift[4], 1u << hand_shift[5], 
                                1u << hand_shift[6] };

inline int pc_to_hp(const Piece pc) {
    assert(piece::is_valid_piece(pc));
    return pc - 1;
}
inline int num(const uint32 hand, const Piece pc) {
    const int hp = pc_to_hp(pc);
    return (hand & hand_mask[hp]) >> hand_shift[hp];
}
inline bool has(const uint32 hand, const Piece pc) {
    const int hp = pc_to_hp(pc);
    return (hand & hand_mask[hp]) != 0;
}

template<bool inc> int hand_change(const uint32 hand, const Piece pc) {
    const int flag = (inc) ? 1 : -1;
    return hand + flag * hand_inc[pc_to_hp(pc)];
}
inline bool hand_is_empty(const uint32 hand) {
    return hand == 0;
}
inline bool is_superior(const uint32 hand1, const uint32 hand2) {
    static constexpr uint32 hand_overflow_mask = hand_mask[0] | hand_mask[1] | hand_mask[2] | hand_mask[3] | hand_mask[4] | hand_mask[5] | hand_mask[6];
    return ((hand1 - hand2) & hand_overflow_mask) == 0;
}

std::string hp_to_string(const uint32 hand);

#endif
