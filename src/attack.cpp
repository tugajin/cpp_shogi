#include "attack.hpp"
#include "pos.hpp"

static bool can_play(const Pos &pos) {
    //todo
    return true;
}
static bool in_check(const Pos &pos, const Side sd) {
    return has_attack(pos,flip_turn(sd),pos.king(sd));
}
bool is_legal(const Pos &pos) {
    return !in_check(pos,flip_turn(pos.turn()));
}
bool is_mate(const Pos &pos) {
    return !can_play(pos);
}
bool in_check(const Pos &pos) {
    return bool(checks(pos));
}
bit::Bitboard checks(const Pos &pos) {

}
bool move_is_safe(const Move mv, const Pos &pos) {
    //todo
    return true;
}
bool move_is_win(const Move mv, const Pos &pos) {
    //todo
    return true;
}

bool has_attack(const Pos &pos, const Side sd, const Square to) {
    return has_attack(pos,sd,to,pos.pieces());
}
bool has_attack(const Pos &pos, const Side sd, const Square to, bit::Bitboard pieces) {

}

bit::Bitboard attacks_to(const Pos &pos, const Side sd, const Square sq, const bit::Bitboard pieces) {
    
}
bit::Bitboard pseudo_attacks_to(const Pos &pos, const Side sd, const Square sq);

bool is_pinned(const Pos &pos, const Square king, const Square sq, const Side sd);
Square pinned_by(const Pos &pos, const Square king, const Square sq, const Side sd);
bit::Bitboard pins(const Pos &pos, const Square king);

bool is_mate_with_pawn_drop(const Square to, const Pos &pos);