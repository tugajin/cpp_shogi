#ifndef NO_GPU

#include "nn.hpp"
#include "../cpp/pos.hpp"
#include "../cpp/sfen.hpp"
#include "../cpp/move.hpp"

enum FeatPos : int { 
    F_HAND_PAWN_POS = 0,
    E_HAND_PAWN_POS = F_HAND_PAWN_POS + 18,
    F_HAND_LANCE_POS = E_HAND_PAWN_POS + 18,
    E_HAND_LANCE_POS = F_HAND_LANCE_POS + 4,
    F_HAND_KNIGHT_POS = E_HAND_LANCE_POS + 4,
    E_HAND_KNIGHT_POS = F_HAND_KNIGHT_POS + 4,
    F_HAND_SILVER_POS = E_HAND_KNIGHT_POS + 4,
    E_HAND_SILVER_POS = F_HAND_SILVER_POS + 4,
    F_HAND_GOLD_POS = E_HAND_SILVER_POS + 4,
    E_HAND_GOLD_POS = F_HAND_GOLD_POS + 4,
    F_HAND_BISHOP_POS = E_HAND_GOLD_POS + 4,
    E_HAND_BISHOP_POS = F_HAND_BISHOP_POS + 2,
    F_HAND_ROOK_POS = E_HAND_BISHOP_POS + 2,
    E_HAND_ROOK_POS = F_HAND_ROOK_POS + 2,
    F_POS_PAWN_POS = E_HAND_ROOK_POS + 2,
    
    E_POS_PAWN_POS,
    F_POS_LANCE_POS,
    E_POS_LANCE_POS,
    F_POS_KNIGHT_POS,
    E_POS_KNIGHT_POS,
    F_POS_SILVER_POS,
    E_POS_SILVER_POS,
    F_POS_GOLD_POS,
    E_POS_GOLD_POS,
    F_POS_BISHOP_POS,
    E_POS_BISHOP_POS,
    F_POS_ROOK_POS,
    E_POS_ROOK_POS,
    F_POS_KING_POS,
    E_POS_KING_POS,
    F_POS_PPAWN_POS,
    E_POS_PPAWN_POS,
    F_POS_PLANCE_POS,
    E_POS_PLANCE_POS,
    F_POS_PKNIGHT_POS,
    E_POS_PKNIGHT_POS,
    F_POS_PSILVER_POS,
    E_POS_PSILVER_POS,
    F_POS_PBISHOP_POS,
    E_POS_PBISHOP_POS,
    F_POS_PROOK_POS,
    E_POS_PROOK_POS,
    POS_END_SIZE,
};

enum MoveClassPos : int { 
    CLS_UP,
    CLS_UL = CLS_UP + SQUARE_SIZE,
    CLS_LF = CLS_UL + SQUARE_SIZE,
    CLS_DL = CLS_LF + SQUARE_SIZE,
    CLS_DW = CLS_DL + SQUARE_SIZE,
    CLS_DR = CLS_DW + SQUARE_SIZE, 
    CLS_RG = CLS_DR + SQUARE_SIZE,
    CLS_UR = CLS_RG + SQUARE_SIZE,
    CLS_L_KNT = CLS_UR + SQUARE_SIZE,
    CLS_R_KNT = CLS_L_KNT + SQUARE_SIZE,
    CLS_UP_PROM = CLS_R_KNT + SQUARE_SIZE,
    CLS_UL_PROM = CLS_UP_PROM + SQUARE_SIZE,
    CLS_LF_PROM = CLS_UL_PROM + SQUARE_SIZE,
    CLS_DL_PROM = CLS_LF_PROM + SQUARE_SIZE,
    CLS_DW_PROM = CLS_DL_PROM + SQUARE_SIZE,
    CLS_DR_PROM = CLS_DW_PROM + SQUARE_SIZE,
    CLS_RG_PROM = CLS_DR_PROM + SQUARE_SIZE,
    CLS_UR_PROM = CLS_RG_PROM + SQUARE_SIZE,
    CLS_L_KNT_PROM = CLS_UR_PROM + SQUARE_SIZE,
    CLS_R_KNT_PROM = CLS_L_KNT_PROM + SQUARE_SIZE,
    CLS_HAND_PAWN = CLS_R_KNT_PROM + SQUARE_SIZE,
    CLS_HAND_LANCE = CLS_HAND_PAWN + SQUARE_SIZE,
    CLS_HAND_KNIGHT = CLS_HAND_LANCE + SQUARE_SIZE,
    CLS_HAND_SILVER = CLS_HAND_KNIGHT + SQUARE_SIZE,
    CLS_HAND_GOLD = CLS_HAND_SILVER + SQUARE_SIZE,
    CLS_HAND_BISHOP = CLS_HAND_GOLD + SQUARE_SIZE,
    CLS_HAND_ROOK = CLS_HAND_BISHOP + SQUARE_SIZE,
    CLS_MOVE_END = CLS_HAND_ROOK + SQUARE_SIZE,
};

TeeStream Tee;

void make_feat(const Pos& pos, float feat[POS_END_SIZE][SQUARE_SIZE]) {
    
    const auto me = pos.turn();
    const auto opp = flip_turn(me);

    SIDE_FOREACH(sd) {
        HAND_FOREACH(hp) {
            const auto num = hand_num(pos.hand(sd), hp);
            if(!num) { continue; }
            auto index = 0;
            switch (hp) {
            case Pawn:
                index = (me == sd) ? F_HAND_PAWN_POS : E_HAND_PAWN_POS;
                break;
            case Lance:
                index = (me == sd) ? F_HAND_LANCE_POS : E_HAND_LANCE_POS;
                break;
            case Knight:
                index = (me == sd) ? F_HAND_KNIGHT_POS : E_HAND_KNIGHT_POS;
                break;
            case Silver:
                index = (me == sd) ? F_HAND_SILVER_POS : E_HAND_SILVER_POS;
                break;
            case Gold:
                index = (me == sd) ? F_HAND_GOLD_POS : E_HAND_GOLD_POS;
                break;
            case Bishop:
                index = (me == sd) ? F_HAND_BISHOP_POS : E_HAND_BISHOP_POS;
                break;
            case Rook:
                index = (me == sd) ? F_HAND_ROOK_POS : E_HAND_ROOK_POS;
                break;
            default:
                assert(false);
                break;
            }
            //重ね合わせで表現してみる
            for(auto all_index = 1; all_index <= num; ++all_index) {
                //feat[king_sq][all_index + index] = torch::ones({ SQUARE_SIZE });
            }
        }
        PIECE_FOREACH(pc) {
            auto piece = pos.pieces(pc, sd);
            while (piece) {
                const auto sq = (me == BLACK) ? piece.lsb() : flip_sq(piece.lsb());
                auto index = 0;
                switch (pc) {
                case Pawn:
                    index = (sd == me) ? F_POS_PAWN_POS : E_POS_PAWN_POS;
                    break;
                case Lance:
                    index = (sd == me) ? F_POS_LANCE_POS : E_POS_LANCE_POS;
                    break;
                case Knight:
                    index = (sd == me) ? F_POS_KNIGHT_POS : E_POS_KNIGHT_POS;
                    break;
                case Silver:
                    index = (sd == me) ? F_POS_SILVER_POS : E_POS_SILVER_POS;
                    break;
                case Gold:
                    index = (sd == me) ? F_POS_GOLD_POS : E_POS_GOLD_POS;
                    break;
                case King:
                    index = (sd == me) ? F_POS_KING_POS : E_POS_KING_POS;
                    break;
                case Bishop:
                    index = (sd == me) ? F_POS_BISHOP_POS : E_POS_BISHOP_POS;
                    break;
                case Rook:
                    index = (sd == me) ? F_POS_ROOK_POS : E_POS_ROOK_POS;
                    break;
                case PPawn:
                    index = (sd == me) ? F_POS_PPAWN_POS : E_POS_PPAWN_POS;
                    break;
                case PLance:
                    index = (sd == me) ? F_POS_PLANCE_POS : E_POS_PLANCE_POS;
                    break;
                case PKnight:
                    index = (sd == me) ? F_POS_PKNIGHT_POS : E_POS_PKNIGHT_POS;
                    break;
                case PSilver:
                    index = (sd == me) ? F_POS_PSILVER_POS : E_POS_PSILVER_POS;
                    break;
                case PBishop:
                    index = (sd == me) ? F_POS_PBISHOP_POS : E_POS_PBISHOP_POS;
                    break;
                case PRook:
                    index = (sd == me) ? F_POS_PROOK_POS : E_POS_PROOK_POS;
                    break;
                default:
                    assert(false);
                    break;
                }
                feat[index][sq] = 1.0;
            }
        }
    }
}

MoveClassPos move_to_index(const Move mv, const Side sd) {
    
    if (move::move_is_drop(mv)) {
        const auto to = (sd == BLACK) ? move::move_to(mv) : flip_sq(move::move_to(mv));
        const auto pc = move::move_piece(mv);
        switch (pc) {
        case Pawn:
            return MoveClassPos(int(CLS_HAND_PAWN) + int(to));
        case Lance:
            return MoveClassPos(int(CLS_HAND_LANCE) + int(to));
        case Knight:
            return MoveClassPos(int(CLS_HAND_KNIGHT) + int(to));
        case Silver:
            return MoveClassPos(int(CLS_HAND_SILVER) + int(to));
        case Gold:
            return MoveClassPos(int(CLS_HAND_GOLD) + int(to));
        case Bishop:
            return MoveClassPos(int(CLS_HAND_BISHOP) + int(to));
        case Rook:
            return MoveClassPos(int(CLS_HAND_ROOK) + int(to));
        default:
            assert(false);
        }
    }
    else {
        const auto from = (sd == BLACK) ? move::move_from(mv) : flip_sq(move::move_from(mv));
        const auto to = (sd == BLACK) ? move::move_to(mv) : flip_sq(move::move_to(mv));
        const auto prom = move::move_is_prom(mv);
        switch (get_direction(from,to)) {
        case DIR_UP:
            return (prom) ? MoveClassPos(int(CLS_UP_PROM) + int(to)) : MoveClassPos(int(CLS_UP) + int(to));
        case DIR_UL:
            return (prom) ? MoveClassPos(int(CLS_UL_PROM) + int(to)) : MoveClassPos(int(CLS_UL) + int(to));
        case DIR_LF:
            return (prom) ? MoveClassPos(int(CLS_LF_PROM) + int(to)) : MoveClassPos(int(CLS_LF) + int(to));
        case DIR_DL:
            return (prom) ? MoveClassPos(int(CLS_DL_PROM) + int(to)) : MoveClassPos(int(CLS_DL) + int(to));
        case DIR_DW:
            return (prom) ? MoveClassPos(int(CLS_DW_PROM) + int(to)) : MoveClassPos(int(CLS_DW) + int(to));
        case DIR_DR:
            return (prom) ? MoveClassPos(int(CLS_DR_PROM) + int(to)) : MoveClassPos(int(CLS_DR) + int(to));
        case DIR_RG:
            return (prom) ? MoveClassPos(int(CLS_RG_PROM) + int(to)) : MoveClassPos(int(CLS_RG) + int(to));
        case DIR_UR:
            return (prom) ? MoveClassPos(int(CLS_UR_PROM) + int(to)) : MoveClassPos(int(CLS_UR) + int(to));
        case DIR_L_KNT:
            return (prom) ? MoveClassPos(int(CLS_L_KNT_PROM) + int(to)) : MoveClassPos(int(CLS_L_KNT) + int(to));
        case DIR_R_KNT:
            return (prom) ? MoveClassPos(int(CLS_R_KNT_PROM) + int(to)) : MoveClassPos(int(CLS_R_KNT) + int(to));
        default:
            assert(false);
        }
    }
}
#endif

int main() {

	bit::init();
	hash::init();
	pos::init();
	common::init();

    Pos pos = pos_from_sfen(START_SFEN);
    std::cout<<pos<<std::endl;
}
