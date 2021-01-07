#ifndef NN_HPP
#define NN_HPP

#include "common.hpp"
#include <boost/python.hpp>

extern TeeStream Tee;

enum FeatPos : int { 
    //持ち駒の枚数
    //歩は上限をもたせる
    //特徴は重ね合わせる
    //歩を3枚持っていたら、1枚、2枚、3枚の特徴が1になる 
    F_HAND_PAWN_POS = 0,
    E_HAND_PAWN_POS = F_HAND_PAWN_POS + 5,
    F_HAND_LANCE_POS = E_HAND_PAWN_POS + 5,
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

    //駒の位置 あれば1 なければ0
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

    //どの駒の直接利きがあるか
    //利きがあれば1なければ0
    F_DIRECT_ATTACK_PAWN_POS,
    E_DIRECT_ATTACK_PAWN_POS,
    F_DIRECT_ATTACK_LANCE_POS,
    E_DIRECT_ATTACK_LANCE_POS,
    F_DIRECT_ATTACK_KNIGHT_POS,
    E_DIRECT_ATTACK_KNIGHT_POS,
    F_DIRECT_ATTACK_SILVER_POS,
    E_DIRECT_ATTACK_SILVER_POS,
    F_DIRECT_ATTACK_GOLD_POS,
    E_DIRECT_ATTACK_GOLD_POS,
    F_DIRECT_ATTACK_BISHOP_POS,
    E_DIRECT_ATTACK_BISHOP_POS,
    F_DIRECT_ATTACK_ROOK_POS,
    E_DIRECT_ATTACK_ROOK_POS,
    F_DIRECT_ATTACK_KING_POS,
    E_DIRECT_ATTACK_KING_POS,
    F_DIRECT_ATTACK_PPAWN_POS,
    E_DIRECT_ATTACK_PPAWN_POS,
    F_DIRECT_ATTACK_PLANCE_POS,
    E_DIRECT_ATTACK_PLANCE_POS,
    F_DIRECT_ATTACK_PKNIGHT_POS,
    E_DIRECT_ATTACK_PKNIGHT_POS,
    F_DIRECT_ATTACK_PSILVER_POS,
    E_DIRECT_ATTACK_PSILVER_POS,
    F_DIRECT_ATTACK_PBISHOP_POS,
    E_DIRECT_ATTACK_PBISHOP_POS,
    F_DIRECT_ATTACK_PROOK_POS,
    E_DIRECT_ATTACK_PROOK_POS,

    //どの駒の間接利きがあるか
    //利きがあれば1なければ0
    F_DISCOVER_ATTACK_LANCE_POS,
    E_DISCOVER_ATTACK_LANCE_POS,
    F_DISCOVER_ATTACK_BISHOP_POS,
    E_DISCOVER_ATTACK_BISHOP_POS,
    F_DISCOVER_ATTACK_ROOK_POS,
    E_DISCOVER_ATTACK_ROOK_POS,

    //利きの数
    //利きの数が1個なら該当の特徴が1
    //特徴は重ね合わせる
    //利きが3つだったら、1、2、3の特徴が1になる 
    F_DIRECT_ATTACK_NUM_POS,
    E_DIRECT_ATTACK_NUM_POS = F_DIRECT_ATTACK_NUM_POS + 3,
    F_DISCOVER_ATTACK_NUM_POS = E_DIRECT_ATTACK_NUM_POS + 3,
    E_DISCOVER_ATTACK_NUM_POS = F_DISCOVER_ATTACK_NUM_POS + 3,

    //利きの勝ち負け
    //利きの数が勝っているか、引き分けか、負けているか
    //この特徴は自分から見ればいいのでEは不要
    F_DIRECT_ATTACK_WINNER_POS = E_DISCOVER_ATTACK_NUM_POS + 3,
    F_DIRECT_ATTACK_LOSER_POS,
    F_DIRECT_ATTACK_DRAW_POS,

    //pin
    //ピンだったら1
    F_PIN_POS,
    E_PIN_POS,
    //玉が動けるかどうか
    //玉が動けるなら１
    F_KING_ESCAPE_POS,
    E_KING_ESCAPE_POS,
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

class Pos;

void make_feat(const Pos& pos, float feat[POS_END_SIZE][FILE_SIZE][RANK_SIZE]);
MoveClassPos move_to_index(const Move mv, const Side sd);
MoveClassPos move_to_index(Square sq_from, Square sq_to, Piece pc, bool prom, Side sd);

namespace nn {
    extern boost::python::object g_main_ns;
    extern boost::python::object g_model;
    extern boost::python::object load_model_func;
    extern boost::python::object forward_func;
    extern boost::python::object my_close_func;
    
    void init();
    void test();
    void my_close();
    
}

#endif
