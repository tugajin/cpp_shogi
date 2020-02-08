#include "pos.hpp"
#include "move.hpp"
#include "hand.hpp"
#include "sfen.hpp"
#include "bit.hpp"

Pos::Pos(Side turn, Bitboard piece_side[], int hand[]) {
    this->clear();
    PIECE_SIDE_FOREACH(ps) {
        auto pc = piece_side_piece(ps);
        auto sd = piece_side_side(ps);
        for(auto bb = piece_side[ps]; bb; ) {
            auto sq = bb.lsb();
            this->add_piece(pc,sd,sq);
        }
    }
    this->hand_[BLACK] = this->hand_[WHITE] = HAND_NONE;
    PIECE_SIDE_FOREACH(ps) {
        auto pc = piece_side_piece(ps);
        auto sd = piece_side_side(ps);
        this->hand_[sd] = hand_change<true>(this->hand_[sd],pc,hand[ps]);
    }
    if(turn == WHITE) {
        this->switch_turn();
    }
    this->update();
}
void Pos::clear() {
    for(auto &piece : this->piece_) {
        piece.init();
    }
    for(auto &sd : this->side_) {
        sd.init();
    }
    this->all_.init();
    this->turn_ = BLACK;
    this->ply_ = 0;
    for(auto & sq : this->square_) {
        sq = PieceNone;
    }
    this->last_move_ = move::MOVE_NONE;
    this->cap_sq_ = SQ_NONE;
    this->key_ = Key(0ULL);
    this->hand_b_ = 0u;

}
Pos Pos::succ(const Move move) const {
    const auto to = move::move_to(move);
    const auto sd = this->turn();
    const auto xd = flip_turn(sd);
    auto pos = *this;
    pos.ply_ = this->ply_+1;
    pos.last_move_ = move;
    const auto pc = move::move_piece(move);
    if(move::move_is_drop(move)) {
        pos.hand_[sd] = hand_change<false>(pos.hand_[sd],pc);
        pos.add_piece(pc,sd,to);
    } else {
        const auto from = move::move_from(move);
        const auto cap = move::move_cap(move);
        const auto prom = move::move_is_prom(move);
        if(cap != PieceNone) {
            pos.remove_piece(cap,xd,to);
            pos.cap_sq_ = to;
            const auto unprom_cap = piece_unprom(cap);
            pos.hand_[sd] = hand_change<true>(pos.hand_[sd],unprom_cap);
        }
        pos.remove_piece(pc,sd,from);
        if(prom) { 
            pos.add_piece(piece_prom(pc),sd,to);
        } else {
            pos.add_piece(pc,sd,to);
        }
    }
    pos.switch_turn();
    pos.update();
    return pos;
}
namespace pos {

Pos gStart;

void init() {
    gStart = pos_from_sfen(START_SFEN);
}

void test() {
    Tee<<"start test pos\n";
    
    Pos p = pos_from_sfen(START_SFEN);
    Tee<<p<<std::endl;
   
    Tee<<"end test pos\n";
    bit::Bitboard bb;

    bb = get_file_attack(SQ_19,p.pieces());
    Tee<<bb<<std::endl;
    p = pos_from_sfen("lnsgkgsnl/1r5b1/ppppppppp/9/9/9/9/4B2R1/LNSGKGSNL b - 1");
    Tee<<p<<std::endl;
    bb = get_file_attack(SQ_19,p.pieces());
    Tee<<"sq19"<<std::endl;
    Tee<<bb<<std::endl;
    bb = get_file_attack(SQ_28,p.pieces());
    Tee<<"sq28"<<std::endl;
    Tee<<bb<<std::endl;
    
    bb = get_file_attack(SQ_99,p.pieces());
    Tee<<"sq99"<<std::endl;
    Tee<<bb<<std::endl;
    bb = get_rank_attack(SQ_28,p.pieces());
    Tee<<"sq28"<<std::endl;
    Tee<<bb<<std::endl;

    Tee<<p<<std::endl;

    bb = get_diag1_attack(SQ_88,p.pieces());
    Tee<<"sq28"<<std::endl;
    Tee<<bb<<std::endl;

    p = pos_from_sfen("4k4/9/9/9/9/9/9/9/4K4 b - 1");
    Tee<<p<<std::endl;

    bb = get_diag1_attack(SQ_55,p.pieces());
    Tee<<"sq55"<<std::endl;
    Tee<<bb<<std::endl;

    bb = get_diag2_attack(SQ_55,p.pieces());
    Tee<<"sq55"<<std::endl;
    Tee<<bb<<std::endl;

    p = pos_from_sfen("4k4/9/ppppppppp/9/9/9/PPPPPPPPP/9/4K4 b - 1");
    Tee<<p<<std::endl;

    bb = get_diag1_attack(SQ_55,p.pieces());
    Tee<<"sq55"<<std::endl;
    Tee<<bb<<std::endl;

    bb = get_diag2_attack(SQ_55,p.pieces());
    Tee<<"sq55"<<std::endl;
    Tee<<bb<<std::endl;


}

}