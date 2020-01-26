#ifndef POS_HPP
#define POS_HPP

#include "common.hpp"
#include "bit.hpp"

using namespace bit;

class Pos {
private:
    Bitboard piece_[PIECE_SIZE];
    Bitboard side_[SIDE_SIZE];
    Bitboard all_;
    Side turn_;
    Piece square_[SQUARE_SIZE];
    Hand hand_[SIDE_SIZE];
    int ply_;
    Move last_move_;
    Square cap_sq_;
    Key key_;
    uint32 hand_b_;
public:
    Pos();
    Pos(Side turn, Bitboard piece_side[]);
    Pos do_move(Move mv) const;
    Side turn() const { return this->turn_; }
    Bitboard empties() const { return ~this->all_; }
    Bitboard pieces() const { return this->all_; }
    Bitboard pieces(Piece pc) const { return this->piece_[pc]; }
    Bitboard pieces(Side sd) const { return this->side_[sd]; }
    Bitboard pieces(Piece pc, Side sd) const { return this->pieces(pc) & this->pieces(sd); }
    Square king(Side sd) const { 
        //TODO
        auto sq = this->pieces(King,sd).lsb<false>();
        return Square(sq);
    }
    bool is_empty(const Square sq) const { return this->square_[sq] == PieceNone; }
    bool is_piece(const Square sq, Piece pc) const { return this->square_[sq] == pc; }
    bool is_side(const Square sq, Side sd) const { return this->side_[sd].is_set(sq); }
    Piece piece(const Square sq) const { return this->square_[sq]; }
    Side side(const Square sq) const { return Side(!this->is_side(sq,BLACK)); }
    Move last_move() const { return this->last_move_; }
    Square cap_sq() const { return this->cap_sq_; }
    Key key() const { return this->key_; }
    uint32 hand_b() const { return this->hand_b_; }
    int ply() const { return this->ply_; }
    Hand hand(const Side sd) const { return this->hand_[sd]; }
private:
    void clear();
    void update();
    void switch_turn();
};

namespace pos {

void init();

}

#endif