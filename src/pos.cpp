#include "pos.hpp"
#include "move.hpp"
#include "hand.hpp"
#include "sfen.hpp"
#include "bit.hpp"
#include "attack.hpp"

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
    assert(is_ok());
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
    this->ply_ = Ply(0);
    for(auto & sq : this->square_) {
        sq = PieceNone;
    }
    this->last_move_ = move::MOVE_NONE;
    this->cap_sq_ = SQ_NONE;
    this->key_ = Key(0ULL);
    this->hand_b_ = 0u;
    this->parent_ = nullptr;

}
Pos Pos::succ(const Move move) const {
    
    assert(is_ok());
    assert(move::move_is_ok(move,*this));
    
    const auto to = move::move_to(move);
    const auto sd = this->turn();
    const auto xd = flip_turn(sd);
    auto pos = *this;
    pos.parent_ = this;
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
    assert(pos.is_ok());
    return pos;
}

bool Pos::is_ok() const {
    if(!hand_is_ok(this->hand_[BLACK])) {
        Tee<<"black hand error";
        return false;
    }
    if(!hand_is_ok(this->hand_[WHITE])) {
        Tee<<"white hand error";
        return false;
    }
    auto tmp_all = this->side_[BLACK] | this->side_[WHITE];
    if(tmp_all != this->all_) {
        Tee<<"all error";
        Tee<<tmp_all<<std::endl;
        Tee<<this->all_<<std::endl;
        return false;
    }
    PIECE_FOREACH(pc) {
        auto bb = this->piece_[pc];
        while(bb) {
            auto sq = bb.lsb();
            if(!this->all_.is_set(sq)) {
                Tee<<"piece -> all error"<<std::endl;
                Tee<<pc<<std::endl;
                Tee<<sq<<std::endl;
                Tee<<this->piece_[pc]<<std::endl;
                return false;
            }
            if(this->square_[sq] != pc) {
                Tee<<"pc error"<<std::endl;
                Tee<<this->square_[sq]<<std::endl;
                Tee<<pc<<std::endl;
                Tee<<sq<<std::endl;
                return false;
            }
        }
    }
    while(tmp_all) {
        const auto sq = tmp_all.lsb();
        if(this->square_[sq] == PieceNone) {
            Tee<<"sq error"<<std::endl;
            Tee<<this->square_[sq]<<std::endl;
            Tee<<sq<<std::endl;
            return false;
        }
    }
    if(this->ply_ < 0) {
        Tee<<"ply error"<<std::endl;
        Tee<<this->ply_<<std::endl;
        return false;
    }
    if(uint32(this->hand_[BLACK]) != this->hand_b_) {
        Tee<<"hand_b error"<<std::endl;
        Tee<<uint32(this->hand_[BLACK])<<std::endl;
        Tee<<this->hand_b_<<std::endl;
        return false;
    }
    auto debug_key = hash::key_turn(this->turn());
    SQUARE_FOREACH(sq){
        const auto pc = this->square_[sq];
        if(pc != PieceNone) {
            const auto sd = this->side(sq);
            debug_key ^= hash::key_piece(pc,sd,sq);
        }
    }
    if(this->key_ != debug_key) {
        Tee<<"key not eq"<<std::endl;
        Tee<<uint64(debug_key)<<std::endl;
        Tee<<uint64(this->key_)<<std::endl;
        return false;
    }
    return true;
}
std::ostream& operator<<(std::ostream& os, const Pos& b){
    if(b.turn() == BLACK) {
        os << "BLACK" << std::endl;
    } else {
        os << "WHITE" << std::endl;
    }
        
    os << "key:" << uint64(b.key()) << std::endl;
    os << "ply:" << b.ply() << std::endl;
    os <<out_sfen(b)<<std::endl;
    os << hand_to_string(b.hand(WHITE)) << std::endl;
    RANK_FOREACH(rank){
        if (!rank) {
            os << "  ";
            FILE_FOREACH_REV(file){
                os <<" "<< file + 1;
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
                os << " .";
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

Pos::RepState Pos::is_draw() const {
    if(this->ply() < 4) {
        return Pos::RepNone;
    }
    auto pos = this;
    for(auto i = 0; i < 4; i++) {
        assert(pos->parent_ != nullptr);
        auto org = pos;
        pos = pos->parent_->parent_;
        assert(org != nullptr);
        assert(pos != nullptr);

        if(pos->key() == this->key()) {
            if(pos->hand_b() == this->hand_b()) {
                const auto xd = flip_turn(this->turn());
                if(in_check(*this) && in_check(*org)) {
                    return Pos::RepChecked;
                } else if(in_check(*org->parent_)) {
                    return Pos::RepCheck;
                } else {
                    return Pos::RepEq;
                } 
            }
            if(hand_is_superior(this->hand_[BLACK],pos->hand_[BLACK])) {
                if(this->turn() == BLACK) {
                    return Pos::RepHandWin;
                } else {
                    return Pos::RepHandLose;
                }
            } 
            if(hand_is_superior(pos->hand_[BLACK],this->hand_[BLACK])) {
                if(this->turn() == BLACK) {
                    return Pos::RepHandLose;
                } else {
                    return Pos::RepHandWin;
                }
            }
        }
    }
    return Pos::RepNone;
}

namespace pos {

Pos gStart;

void init() {
    gStart = pos_from_sfen(START_SFEN);
}

static void test_gen() {
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

void test_draw() {
    {
        auto p = pos_from_sfen("9/9/ppppppppp/4k4/1R7/3PP4/9/9/4K4 b - 1");
        
        auto mv1 = move::from_usi("8e5e",p);
        auto p1 =  p.succ(mv1);
        Tee<<p1<<std::endl;;

        auto mv2 = move::from_usi("5d6d",p1);
        auto p2 =  p1.succ(mv2);
        Tee<<p2<<std::endl;;

        auto mv3 = move::from_usi("5e6e",p2);
        auto p3 =  p2.succ(mv3);
        Tee<<p3<<std::endl;;

        auto mv4 = move::from_usi("6d5d",p3);
        auto p4 =  p3.succ(mv4);
        Tee<<p4<<std::endl;;
        
        auto mv5 = move::from_usi("6e5e",p4);
        auto p5 =  p4.succ(mv5);
        Tee<<p5<<std::endl;;
        Tee<<p5.is_draw()<<std::endl;
        assert(p5.is_draw() == Pos::RepChecked);
        auto mv6 = move::from_usi("5d6d",p5);
        auto p6 =  p5.succ(mv6);
        Tee<<p6<<std::endl;;
    
        Tee<<p6.is_draw()<<std::endl;
        assert(p6.is_draw() == Pos::RepCheck);
    }
    {
        auto p = pos_from_sfen("4k4/9/9/9/9/9/9/9/4K4 b - 1");
        
        auto mv1 = move::from_usi("5i5h",p);
        auto p1 =  p.succ(mv1);
        Tee<<p1<<std::endl;;
        
        auto mv2 = move::from_usi("5a5b",p1);
        auto p2 =  p1.succ(mv2);
        Tee<<p2<<std::endl;;

        auto mv3 = move::from_usi("5h5i",p2);
        auto p3 =  p2.succ(mv3);
        Tee<<p3<<std::endl;;

        auto mv4 = move::from_usi("5b5a",p3);
        auto p4 =  p3.succ(mv4);
        Tee<<p4<<std::endl;;
        Tee<<p4.is_draw()<<std::endl;;
        assert(p4.is_draw() == Pos::RepEq);
        
    }
   {
        auto p = pos_from_sfen("4k4/9/9/9/9/9/9/9/4K4 b 3G - 1");
        
        auto mv1 = move::from_usi("G*6a",p);
        auto p1 =  p.succ(mv1);
        Tee<<p1<<std::endl;;
        
        auto mv2 = move::from_usi("5a6a",p1);
        auto p2 =  p1.succ(mv2);
        Tee<<p2<<std::endl;;

        auto mv3 = move::from_usi("G*5a",p2);
        auto p3 =  p2.succ(mv3);
       
        auto mv4 = move::from_usi("6a5a",p3);
        auto p4 =  p3.succ(mv4);
        Tee<<p4<<std::endl;;
        Tee<<p4.is_draw()<<std::endl;;
        assert(p4.is_draw() == Pos::RepHandLose);
        
        auto mv5 = move::from_usi("G*6a",p4);
        auto p5 =  p4.succ(mv5);
        Tee<<p5<<std::endl;
        assert(p5.is_draw() == Pos::RepHandWin);

        
    }
}

void test() {
    Tee<<"start test pos\n";
    //test_gen();    
    test_draw();
}

}