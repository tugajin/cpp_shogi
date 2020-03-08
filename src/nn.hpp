#ifndef NN_HPP
#define NN_HPP

#include "common.hpp"
#include "hand.hpp"
#include <torch/torch.h>

constexpr int DIRECTION_SIZE{ 10 };
constexpr int HAND_FEAT_SIZ{ 10 };

enum FeatPos : int { 
    F_HAND_PAWN_POS = 0,
    E_HAND_PAWN_POS = F_HAND_PAWN_POS + 19,
    F_HAND_LANCE_POS = E_HAND_PAWN_POS + 19,
    E_HAND_LANCE_POS = F_HAND_LANCE_POS + 5,
    F_HAND_KNIGHT_POS = E_HAND_LANCE_POS + 5,
    E_HAND_KNIGHT_POS = F_HAND_KNIGHT_POS + 5,
    F_HAND_SILVER_POS = E_HAND_KNIGHT_POS + 5,
    E_HAND_SILVER_POS = F_HAND_SILVER_POS + 5,
    F_HAND_GOLD_POS = E_HAND_SILVER_POS + 5,
    E_HAND_GOLD_POS = F_HAND_GOLD_POS + 5,
    F_HAND_BISHOP_POS = E_HAND_GOLD_POS + 5,
    E_HAND_BISHOP_POS = F_HAND_BISHOP_POS + 3,
    F_HAND_ROOK_POS = E_HAND_BISHOP_POS + 3,
    E_HAND_ROOK_POS = F_HAND_ROOK_POS + 3,
    F_POS_PAWN_POS = E_HAND_ROOK_POS + 3,
    E_POS_PAWN_POS = F_POS_PAWN_POS + 81,
    F_POS_LANCE_POS = E_POS_PAWN_POS + 81,
    E_POS_LANCE_POS = F_POS_LANCE_POS + 81,
    F_POS_KNIGHT_POS = E_POS_LANCE_POS + 81,
    E_POS_KNIGHT_POS = F_POS_KNIGHT_POS + 81,
    F_POS_SILVER_POS = E_POS_KNIGHT_POS + 81,
    E_POS_SILVER_POS = F_POS_SILVER_POS + 81,
    F_POS_GOLD_POS = E_POS_SILVER_POS + 81,
    E_POS_GOLD_POS = F_POS_GOLD_POS + 81,
    F_POS_BISHOP_POS = E_POS_GOLD_POS + 81,
    E_POS_BISHOP_POS = F_POS_BISHOP_POS + 81,
    F_POS_ROOK_POS = E_POS_BISHOP_POS + 81,
    E_POS_ROOK_POS = F_POS_ROOK_POS + 81,
    F_POS_KING_POS = E_POS_ROOK_POS + 81,
    E_POS_KING_POS = F_POS_KING_POS + 81,
    F_POS_PPAWN_POS = E_POS_KING_POS + 81,
    E_POS_PPAWN_POS = F_POS_PPAWN_POS + 81,
    F_POS_PLANCE_POS = E_POS_PPAWN_POS + 81,
    E_POS_PLANCE_POS = F_POS_PLANCE_POS + 81,
    F_POS_PKNIGHT_POS = E_POS_PLANCE_POS + 81,
    E_POS_PKNIGHT_POS = F_POS_PKNIGHT_POS + 81,
    F_POS_PSILVER_POS = E_POS_PKNIGHT_POS + 81,
    E_POS_PSILVER_POS = F_POS_PSILVER_POS + 81,
    F_POS_PBISHOP_POS = E_POS_PSILVER_POS + 81,
    E_POS_PBISHOP_POS = F_POS_PBISHOP_POS + 81,
    F_POS_PROOK_POS = E_POS_PBISHOP_POS + 81,
    E_POS_PROOK_POS = F_POS_PROOK_POS + 81,
    F_POS_END_SIZE = E_POS_PROOK_POS + 81,
    E_POS_END_SIZE = F_POS_END_SIZE + 81,
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
    CLS_END = CLS_HAND_ROOK + SQUARE_SIZE,
};

class Pos;

class NNFeat{
public:
    torch::Tensor feat_;

    NNFeat() {
       this->feat_  = torch::zeros({ SIDE_SIZE, E_POS_END_SIZE });
    }
};

void make_feat(const Pos &pos, NNFeat &f);
MoveClassPos move_to_index(const Move mv, const Side sd);
void learn();

namespace nn { 
    void test(); 
}

#endif