#include "attack.hpp"
#include "pos.hpp"

static bool can_play(const Pos &/*pos*/) {
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
template<bool skip_king, bool skip_pawn>bit::Bitboard attacks_to(const Pos &pos, const Side sd, const Square sq, const bit::Bitboard pieces) {
    Bitboard bb(0ull,0ull);
    Bitboard attack,piece;
    const auto xd = flip_turn(sd);
    if(skip_pawn) {
        //pawn
        piece = pos.pieces(Pawn,sd);
        attack = get_pawn_attack(xd,sq);
        bb |= piece & attack;
    }
    //knight
    piece = pos.pieces(Knight,sd);
    attack = get_knight_attack(xd,sq);
    bb |= piece & attack;
    //silver
    piece = pos.pieces(Silver,sd);
    attack = get_knight_attack(xd,sq);
    bb |= piece & attack;
    //gold
    piece = pos.golds(sd);
    attack = get_gold_attack(xd,sq);
    bb |= piece & attack;
    //king
    if(skip_king) {
        piece = (pos.pieces(King) | pos.pieces(PRook) | pos.pieces(PBishop)) & pos.pieces(sd);
    } else {
        piece = (pos.pieces(PRook) | pos.pieces(PBishop)) & pos.pieces(sd);
    }
    attack = get_king_attack(sq);
    bb |= piece & attack;
    //rook rank
    piece = (pos.pieces(Rook) | pos.pieces(PRook)) & pos.pieces(sd);
    attack = get_rank_attack(sq,pieces);
    bb |= piece & attack;
    //rook lance file
    piece |= pos.pieces(Lance,sd) & g_lance_mask[xd][sq];
    attack = get_file_attack(sq,pieces);
    bb |= piece & attack;
    //bishop
    piece = (pos.pieces(Bishop) | pos.pieces(PBishop)) & pos.pieces(sd);
    attack = get_bishop_attack(sq,pieces);
    bb |= piece & attack;

    return bb;
}
bit::Bitboard checks(const Pos &pos) {
    const auto xd = pos.turn();
    const auto sd = flip_turn(xd);
    const auto king = pos.king(xd);
    const auto pieces = pos.pieces();
    return attacks_to<false,false>(pos,sd,king,pieces);
}
bool move_is_safe(const Move /*mv*/, const Pos &/*pos*/) {
    //todo
    return true;
}
bool move_is_win(const Move /*mv*/, const Pos &/*pos*/) {
    //todo
    return true;
}

bool has_attack(const Pos &pos, const Side sd, const Square sq) {
    return has_attack(pos,sd,sq,pos.pieces());
}
bool has_attack(const Pos &pos, const Side sd, const Square sq, bit::Bitboard pieces) {
    Bitboard attack,piece;
    const auto xd = flip_turn(sd);
    //gold
    piece = pos.golds(sd);
    attack = get_gold_attack(xd,sq);
    if((piece & attack)) { return true; }
    //silver
    piece = pos.pieces(Silver,sd);
    attack = get_knight_attack(xd,sq);
    if((piece & attack)) { return true; }
    //king
    piece = (pos.pieces(King) | pos.pieces(PRook) | pos.pieces(PBishop)) & pos.pieces(sd);
    attack = get_king_attack(sq);
    if((piece & attack)) { return true; }
    //rook rank
    piece = (pos.pieces(Rook) | pos.pieces(PRook)) & pos.pieces(sd);
    attack = get_rank_attack(sq,pieces);
    if((piece & attack)) { return true; }
    //rook lance file
    piece |= pos.pieces(Lance,sd) & g_lance_mask[xd][sq];
    attack = get_file_attack(sq,pieces);
    if((piece & attack)) { return true; }
    //bishop
    piece = (pos.pieces(Bishop) | pos.pieces(PBishop)) & pos.pieces(sd);
    attack = get_bishop_attack(sq,pieces);
    if((piece & attack)) { return true; }

    //knight
    piece = pos.pieces(Knight,sd);
    attack = get_knight_attack(xd,sq);
    if((piece & attack)) { return true; }

    //pawn
    piece = pos.pieces(Pawn,sd);
    attack = get_pawn_attack(xd,sq);
    if((piece & attack)) { return true; }
    
    return false;
}

bool is_pinned(const Pos &/*pos*/, const Square /*king*/, const Square /*sq*/, const Side /*sd*/) {
    return false;
}
Square pinned_by(const Pos &/*pos*/, const Square /*king*/, const Square /*sq*/, const Side /*sd*/) {

    return SQ_11;
}
bit::Bitboard pins(const Pos &/*pos*/, const Square /*king*/) {
    return Bitboard(0,0);
}

template<Side sd>bool is_mate_with_pawn_drop(const Square to, const Pos &pos) {
    return false;
}
bool is_mate_with_pawn_drop(const Square to, const Pos &pos) {
  return pos.turn() == BLACK ? is_mate_with_pawn_drop<BLACK>(to,pos)
                             : is_mate_with_pawn_drop<WHITE>(to,pos);
}

template bool is_mate_with_pawn_drop<BLACK>(const Square to, const Pos &pos);
template bool is_mate_with_pawn_drop<WHITE>(const Square to, const Pos &pos);

template bit::Bitboard attacks_to<true,true>(const Pos &pos, const Side sd, const Square sq, const bit::Bitboard pieces);
template bit::Bitboard attacks_to<true,false>(const Pos &pos, const Side sd, const Square sq, const bit::Bitboard pieces);
template bit::Bitboard attacks_to<false,true>(const Pos &pos, const Side sd, const Square sq, const bit::Bitboard pieces);
template bit::Bitboard attacks_to<false,false>(const Pos &pos, const Side sd, const Square sq, const bit::Bitboard pieces);
