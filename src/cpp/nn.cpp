#include <cstring>

#include "nn.hpp"
#include "pos.hpp"
#include "sfen.hpp"
#include "move.hpp"
#include "attack.hpp"
#include "common.hpp"

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

    //pos
    SIDE_FOREACH(sd) {
        HAND_FOREACH(hp) {
            auto num = hand_num(pos.hand(sd), hp);
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
            //4枚以上は同じ扱い
            num = std::min(num, 4);
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

    //盤上の直接利き
    //どこに誰の利きが聞いているか？
    //自分と敵の利きの数
    
    //間接利き
    const auto occ = pos.pieces();
    int direct_attack_num_per_piece[SIDE_SIZE][PIECE_SIZE][SQUARE_SIZE] = {};
    int direct_attack_num[SIDE_SIZE][SQUARE_SIZE] = {};
    int discover_attack_num_per_piece[SIDE_SIZE][PIECE_SIZE][SQUARE_SIZE] = {};
    int discover_attack_num[SIDE_SIZE][SQUARE_SIZE] = {};

    SIDE_FOREACH(sd) {
        PIECE_FOREACH(pc) {
            //直接、間接利きを数える
            auto piece_bb = pos.pieces(pc, sd);
            for (auto piece_bb = pos.pieces(pc, sd); piece_bb;) {
                const auto from = piece_bb.lsb();
                for(auto att_bb = attack_from(pos, sd, from, pc, occ); att_bb; ) {
                    const auto to = att_bb.lsb();
                    direct_attack_num_per_piece[sd][pc][to]++;
                    direct_attack_num[sd][to]++;
                }
                for(auto discover_bb = discover_attacks(from, pos); discover_bb; ) {
                    const auto to = discover_bb.lsb();
                    assert(is_slider(pc));
                    discover_attack_num_per_piece[sd][pc][to]++;
                    discover_attack_num[sd][to]++;
                }
            }
        }
    }
    //利きの数、勝ち負け
    SQUARE_FOREACH(sq) {
        auto me_attack_num = direct_attack_num[me][sq];
        auto opp_attack_num = direct_attack_num[opp][sq];
        const auto file = square_file(sq);
        const auto rank = square_rank(sq);
        //勝ち負け
        if(me_attack_num > opp_attack_num) {
            feat[F_DIRECT_ATTACK_WINNER_POS][file][rank] = 1.0;
        } else if (me_attack_num < opp_attack_num) {
            feat[F_DIRECT_ATTACK_LOSER_POS][file][rank] = 1.0;
        } else {
            feat[F_DIRECT_ATTACK_DRAW_POS][file][rank] = 1.0;
        }
        me_attack_num = std::min(me_attack_num, 3);
        opp_attack_num = std::min(opp_attack_num, 3);
        //利きの数
        for(auto num = 0; num < me_attack_num; num++) {
            feat[F_DIRECT_ATTACK_NUM_POS + num][file][rank] = 1.0;
        }
        for(auto num = 0; num < opp_attack_num; num++) {
            feat[E_DIRECT_ATTACK_NUM_POS + num][file][rank] = 1.0;
        }
        //間接利きの数
        auto me_discover_attack_num = discover_attack_num[me][sq];
        auto opp_discover_attack_num = discover_attack_num[opp][sq];
        me_discover_attack_num = std::min(me_discover_attack_num, 3);
        opp_discover_attack_num = std::min(opp_discover_attack_num, 3);
        for(auto num = 0; num < me_discover_attack_num; num++) {
            feat[F_DISCOVER_ATTACK_NUM_POS + num][file][rank] = 1.0;
        }
        for(auto num = 0; num < opp_discover_attack_num; num++) {
            feat[E_DISCOVER_ATTACK_NUM_POS + num][file][rank] = 1.0;
        }
        //各駒の直接利きがあるか？
        #define TO_FEAT_DIRECT_ATT(piece1, piece2) do { \
        if (direct_attack_num_per_piece[me][piece1][sq]) {\
            feat[F_DIRECT_ATTACK_##piece2##_POS][file][rank] = 1.0;\
        }\
        if (direct_attack_num_per_piece[opp][piece1][sq]) {\
            feat[E_DIRECT_ATTACK_##piece2##_POS][file][rank] = 1.0;\
        }}while(0)\

        TO_FEAT_DIRECT_ATT(Pawn,PAWN);
        TO_FEAT_DIRECT_ATT(Lance,LANCE);
        TO_FEAT_DIRECT_ATT(Knight,KNIGHT);
        TO_FEAT_DIRECT_ATT(Silver,SILVER);
        TO_FEAT_DIRECT_ATT(Gold,GOLD);
        TO_FEAT_DIRECT_ATT(Bishop,BISHOP);
        TO_FEAT_DIRECT_ATT(Rook,ROOK);
        TO_FEAT_DIRECT_ATT(King,KING);
        TO_FEAT_DIRECT_ATT(PPawn,PPAWN);
        TO_FEAT_DIRECT_ATT(PLance,PLANCE);
        TO_FEAT_DIRECT_ATT(PKnight,PKNIGHT);
        TO_FEAT_DIRECT_ATT(PSilver,PSILVER);
        TO_FEAT_DIRECT_ATT(PBishop,PBISHOP);
        TO_FEAT_DIRECT_ATT(PRook,PROOK);

        #undef TO_FEAT_DIRECT_ATT

        //各駒の間接利きがあるか？
        #define TO_FEAT_DISCOVER_ATT(piece1, piece2) do { \
        if (discover_attack_num_per_piece[me][piece1][sq]) {\
            feat[F_DISCOVER_ATTACK_##piece2##_POS][file][rank] = 1.0;\
        }\
        if (direct_attack_num_per_piece[opp][piece1][sq]) {\
            feat[E_DISCOVER_ATTACK_##piece2##_POS][file][rank] = 1.0;\
        }}while(0)\

        TO_FEAT_DISCOVER_ATT(Lance,LANCE);
        TO_FEAT_DISCOVER_ATT(Bishop,BISHOP);
        TO_FEAT_DISCOVER_ATT(Rook,ROOK);
        TO_FEAT_DISCOVER_ATT(PBishop,BISHOP);
        TO_FEAT_DISCOVER_ATT(PRook,ROOK);

        #undef TO_FEAT_DISCOVER_ATT

    }

    //pinされているコマ(自分)
    for(auto pinned_bb_me = pins(pos, me) & pos.pieces(me); pinned_bb_me; ) {
        const auto sq = pinned_bb_me.lsb();
        const auto file = square_file(sq);
        const auto rank = square_rank(sq);
        feat[F_PIN_POS][file][rank] = 1.0;
    }

    //pinになっているコマ（敵）
    for(auto pinned_bb_opp = pins(pos, opp) & pos.pieces(opp); pinned_bb_opp; ) {
        const auto sq = pinned_bb_opp.lsb();
        const auto file = square_file(sq);
        const auto rank = square_rank(sq);
        feat[E_PIN_POS][file][rank] = 1.0;
    }

    //王が動けるかどうか
    SIDE_FOREACH(sd) {
        const auto king_sq = pos.king(sd);
        const auto xd = flip_turn(sd);
        auto index = (sd == me) ? F_KING_ESCAPE_POS : E_KING_ESCAPE_POS;
        auto occ = pos.pieces(sd);
        occ.clear(king_sq);
        occ = ~occ;
        for(auto att_bb = get_king_attack(king_sq) & occ; att_bb; ) {
            const auto to = att_bb.lsb();
            if(!has_attack(pos, xd, to, occ)) {
                const auto file = square_file(to);
                const auto rank = square_rank(to);
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
void disp_feat(const int index, const float feat[POS_END_SIZE][FILE_SIZE][RANK_SIZE]) {
    RANK_FOREACH(r) {
        FILE_FOREACH_REV(f) {
            Tee<<feat[index][f][r]<<",";
        }
        Tee<<std::endl;
    }
}
namespace nn {

    void test() {
        {
			Pos pos = pos_from_sfen("8k/4b4/9/4P4/9/9/4L4/1S2RR1G1/8K b");
			Tee << pos << std::endl;
			float feat[POS_END_SIZE][FILE_SIZE][RANK_SIZE] = {};
            make_feat(pos, feat);
            disp_feat(F_KING_ESCAPE_POS,feat);
		}
        {
			Pos pos = pos_from_sfen(START_SFEN);
			Tee << pos << std::endl;
			float feat[POS_END_SIZE][FILE_SIZE][RANK_SIZE] = {};
            make_feat(pos, feat);
            disp_feat(F_DIRECT_ATTACK_PAWN_POS,feat);
		}
        {
			Pos pos = pos_from_sfen(START_SFEN);
			Tee << pos << std::endl;
			float feat[POS_END_SIZE][FILE_SIZE][RANK_SIZE] = {};
            make_feat(pos, feat);
            disp_feat(F_DISCOVER_ATTACK_ROOK_POS,feat);
		}
    }

}
