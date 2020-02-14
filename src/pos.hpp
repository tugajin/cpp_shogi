#ifndef POS_HPP
#define POS_HPP

#include "common.hpp"
#include "bit.hpp"
#include "hand.hpp"
#include "hash.hpp"

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
    Pos(){};
    Pos(Side turn, Bitboard piece_side[], int hand[]);
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
    Bitboard golds(Side sd) const {
        return (pieces(sd) & (pieces(Gold) | pieces(PPawn) | pieces(PLance) | pieces(PKnight) | pieces(PSilver)));
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
    Pos succ(const Move move)const;
    friend std::ostream& operator<<(std::ostream& os, const Pos& b) {
        if(b.turn() == BLACK) {
            os << "BLACK" << std::endl;
        } else {
            os << "WHITE" << std::endl;
        }
        
        os << "key:" << uint64(b.key()) << std::endl;
        os << "ply:" << b.ply() << std::endl;
        //os <<b.out_sfen()<<std::endl;
        os << hand_to_string(b.hand(WHITE)) << std::endl;
        RANK_FOREACH(rank){
            if (!rank) {
                os << "  ";
                FILE_FOREACH_REV(file){
                    os << file + 1 << " ";
                }
                os << "\n";
            }
            FILE_FOREACH_REV(file){
                if (file == FILE_SIZE - 1) {
                    os << char(rank + 'a') << ":";
                }
                const auto sq = square_make(file, rank);
                const auto pc = b.piece(sq);
                if (pc == PieceNone) {
                    os << ". ";
                } else {
                    const auto sd = b.side(sq);
                    const auto p32 = piece_side_make(pc, sd);
                    os << piece_side_to_sfen(p32);
                }
            }
            os << "\n";
        }
    os << hand_to_string(b.hand(BLACK)) << std::endl;
    return os;
}

private:
    void clear();
    void update() {
        this->all_ = this->side_[BLACK] | this->side_[WHITE];
        this->hand_b_ = hand_to_val(this->hand_[BLACK]);
    }
    void switch_turn() {
        this->turn_ = flip_turn(this->turn_);
        this->key_ ^= hash::key_turn();
    }
    void add_piece(const Piece pc, const Side sd, const Square sq) {
        this->piece_[pc].set(sq);
        this->side_[sd].set(sq);
        this->square_[sq] = pc;
        this->key_ ^= hash::key_piece(pc,sd,sq);
    }
    void remove_piece(const Piece pc, const Side sd, const Square sq) {
        this->piece_[pc].clear(sq);
        this->side_[sd].clear(sq);
        this->square_[sq] = PieceNone;
        this->key_ ^= hash::key_piece(pc,sd,sq);
    }
};

namespace pos {

extern Pos gStart;

void init();
void test();
}

#endif