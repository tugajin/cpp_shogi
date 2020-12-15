#include <cstring>

#include "nn.hpp"
#include "pos.hpp"
#include "sfen.hpp"
#include "move.hpp"

#include <boost/python/numpy.hpp>

TeeStream Tee;

constexpr float ALL_ZERO_NN[FILE_SIZE][RANK_SIZE] = { {0,0,0,0,0,0,0,0,0},
                                                      {0,0,0,0,0,0,0,0,0},
                                                      {0,0,0,0,0,0,0,0,0},
                                                      {0,0,0,0,0,0,0,0,0},
                                                      {0,0,0,0,0,0,0,0,0},
                                                      {0,0,0,0,0,0,0,0,0},
                                                      {0,0,0,0,0,0,0,0,0},
                                                      {0,0,0,0,0,0,0,0,0},
                                                      {0,0,0,0,0,0,0,0,0}};
                                                      
constexpr float ALL_ONE_NN[FILE_SIZE][RANK_SIZE] = { {1,1,1,1,1,1,1,1,1},
                                                     {1,1,1,1,1,1,1,1,1},
                                                     {1,1,1,1,1,1,1,1,1},
                                                     {1,1,1,1,1,1,1,1,1},
                                                     {1,1,1,1,1,1,1,1,1},
                                                     {1,1,1,1,1,1,1,1,1},
                                                     {1,1,1,1,1,1,1,1,1},
                                                     {1,1,1,1,1,1,1,1,1},
                                                     {1,1,1,1,1,1,1,1,1}};

void make_feat(const Pos& pos, float feat[POS_END_SIZE][FILE_SIZE][RANK_SIZE]) {
    
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
            for(auto all_index = 0; all_index < num; ++all_index) {
                std::memcpy(feat[all_index+index],ALL_ONE_NN,sizeof(ALL_ONE_NN));
            }
        }
        PIECE_FOREACH(pc) {
            auto piece = pos.pieces(pc, sd);
            while (piece) {
                const auto sq = (me == BLACK) ? piece.lsb() : flip_sq(piece.lsb());
                const auto file = square_file(sq);
                const auto rank = square_rank(sq);
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
                feat[index][file][rank] = 1.0;
            }
        }
    }
}
MoveClassPos move_to_index(Square sq_from, Square sq_to, Piece pc, bool prom, Side sd) {
   
    if (sq_from >= SQUARE_SIZE) {
        const auto to = (sd == BLACK) ? sq_to : flip_sq(sq_to);
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
            return MoveClassPos(0);
        }
    }
    else {
        const auto from = (sd == BLACK) ? sq_from : flip_sq(sq_from);
        const auto to = (sd == BLACK) ? sq_to : flip_sq(sq_to);
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
            return MoveClassPos(0); 
        }
    }
}
MoveClassPos move_to_index(const Move mv, const Side sd) {
    const auto from = move::move_from(mv);
    const auto to = move::move_to(mv);
    const auto piece = move::move_piece(mv);
    const auto prom = move::move_is_prom(mv);
    return move_to_index(from, to, piece, prom, sd);
}


