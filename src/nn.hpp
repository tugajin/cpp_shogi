#ifndef NN_HPP
#define NN_HPP

#include "common.hpp"
#include "hand.hpp"
#include "pos.hpp"
#include "move.hpp"
#include <torch/torch.h>

constexpr int DIRECTION_SIZE{ 10 };
constexpr int HAND_FEAT_SIZ{ 10 };

constexpr int HIDDEN_NUM{ 256 };
constexpr int BATCH_SIZE{ 32 };

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
    
    /*E_POS_PAWN_POS = F_POS_PAWN_POS + SQUARE_SIZE + 1,
    F_POS_LANCE_POS = E_POS_PAWN_POS + SQUARE_SIZE + 1,
    E_POS_LANCE_POS = F_POS_LANCE_POS + SQUARE_SIZE + 1,
    F_POS_KNIGHT_POS = E_POS_LANCE_POS + SQUARE_SIZE + 1,
    E_POS_KNIGHT_POS = F_POS_KNIGHT_POS + SQUARE_SIZE + 1,
    F_POS_SILVER_POS = E_POS_KNIGHT_POS + SQUARE_SIZE + 1,
    E_POS_SILVER_POS = F_POS_SILVER_POS + SQUARE_SIZE + 1,
    F_POS_GOLD_POS = E_POS_SILVER_POS + SQUARE_SIZE + 1,
    E_POS_GOLD_POS + SQUARE_SIZE + 1,
    F_POS_BISHOP_POS + SQUARE_SIZE + 1,
    E_POS_BISHOP_POS + SQUARE_SIZE + 1,
    F_POS_ROOK_POS + SQUARE_SIZE + 1,
    E_POS_ROOK_POS + SQUARE_SIZE + 1,
    F_POS_KING_POS + SQUARE_SIZE + 1,
    E_POS_KING_POS + SQUARE_SIZE + 1,
    F_POS_PPAWN_POS + SQUARE_SIZE + 1,
    E_POS_PPAWN_POS + SQUARE_SIZE + 1,
    F_POS_PLANCE_POS + SQUARE_SIZE + 1,
    E_POS_PLANCE_POS + SQUARE_SIZE + 1,
    F_POS_PKNIGHT_POS + SQUARE_SIZE + 1,
    E_POS_PKNIGHT_POS + SQUARE_SIZE + 1,
    F_POS_PSILVER_POS + SQUARE_SIZE + 1,
    E_POS_PSILVER_POS + SQUARE_SIZE + 1,
    F_POS_PBISHOP_POS + SQUARE_SIZE + 1,
    E_POS_PBISHOP_POS + SQUARE_SIZE + 1,
    F_POS_PROOK_POS + SQUARE_SIZE + 1,
    E_POS_PROOK_POS + SQUARE_SIZE + 1,
    POS_END_SIZE = E_POS_PROOK_POS + SQUARE_SIZE + 1,*/
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


class NNFeat{
public:
    torch::Tensor feat_;
    torch::Tensor policy_target_;
    torch::Tensor value_target_;
    ml::Array<Pos, BATCH_SIZE> pos_list_;
    ml::Array<Move, BATCH_SIZE> move_list_;

    NNFeat() {
        this->clear();
    }
    void clear() {
        this->feat_ = torch::zeros({ BATCH_SIZE, POS_END_SIZE, SQUARE_SIZE }, torch::TensorOptions().dtype(torch::kFloat));
        this->policy_target_ = torch::zeros({ BATCH_SIZE }, torch::TensorOptions().dtype(torch::kLong));
        this->value_target_ = torch::zeros({ BATCH_SIZE}, torch::TensorOptions().dtype(torch::kFloat));
        pos_list_.clear();
        move_list_.clear();
    }
};



struct NetImpl : torch::nn::Module {
    NetImpl()
        :
        conv1(torch::nn::Conv2dOptions(POS_END_SIZE, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv2(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv3(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv4(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv5(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv6(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv7(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv8(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv9(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv10(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv11(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv12(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv13(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv14(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv15(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),
        conv16(torch::nn::Conv2dOptions(HIDDEN_NUM, HIDDEN_NUM, /*kernel_size=*/3).padding(1)),

        conv_p1(torch::nn::Conv2dOptions(HIDDEN_NUM, 512, /*kernel_size=*/2)),
        conv_v1(torch::nn::Conv2dOptions(HIDDEN_NUM, 1, /*kernel_size=*/1)),

        bn1(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn2(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn3(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn4(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn5(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn6(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn7(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn8(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn9(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn10(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn11(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn12(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn13(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn14(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn15(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),
        bn16(torch::nn::BatchNorm2dOptions(HIDDEN_NUM)),

        bn_p1(torch::nn::BatchNorm2dOptions(512)),
        bn_v1(torch::nn::BatchNorm2dOptions(1)),

        fc_v2(SQUARE_SIZE, HIDDEN_NUM),
        fc_v3(HIDDEN_NUM, 1),
        fc_p2(8 * 8 * 512, CLS_MOVE_END)

    {
        register_module("conv1", conv1);
        register_module("conv2", conv2);
        register_module("conv3", conv3);
        register_module("conv4", conv4);
        register_module("conv5", conv5);
        register_module("conv6", conv6);
        register_module("conv7", conv7);
        register_module("conv8", conv8);
        register_module("conv9", conv9);
        register_module("conv10", conv10);
        register_module("conv11", conv11);
        register_module("conv12", conv12);
        register_module("conv13", conv13);
        register_module("conv14", conv14);
        register_module("conv15", conv15);
        register_module("conv16", conv16);

        register_module("conv_p1", conv_p1);
        register_module("conv_v1", conv_v1);

        register_module("bn1", bn1);
        register_module("bn2", bn2);
        register_module("bn3", bn3);
        register_module("bn4", bn4);
        register_module("bn5", bn5);
        register_module("bn6", bn6);
        register_module("bn7", bn7);
        register_module("bn8", bn8);
        register_module("bn9", bn9);
        register_module("bn10", bn10);
        register_module("bn11", bn11);
        register_module("bn12", bn12);
        register_module("bn13", bn13);
        register_module("bn14", bn14);
        register_module("bn15", bn15);
        register_module("bn16", bn16);

        register_module("bn_p1", bn_p1);
        register_module("bn_v1", bn_v1);

        register_module("fc_v2", fc_v2);
        register_module("fc_v3", fc_v3);
        register_module("fc_p2", fc_p2);
    }

    std::tuple<torch::Tensor,torch::Tensor> forward(torch::Tensor x) {

        auto h1 = torch::relu(bn1(conv1->forward(x)));
        auto h2 = torch::relu(bn2(conv2->forward(h1)));
        auto h3 = torch::relu(bn3(conv3->forward(h2)) + h1);
        auto h4 = torch::relu(bn4(conv4->forward(h3)));
        auto h5 = torch::relu(bn5(conv5->forward(h4)) + h3);
        auto h6 = torch::relu(bn6(conv6->forward(h5)));
        auto h7 = torch::relu(bn7(conv7->forward(h6)) + h5);
        auto h8 = torch::relu(bn8(conv8->forward(h7)));
        auto h9 = torch::relu(bn9(conv9->forward(h8)) + h7);
        auto h10 = torch::relu(bn10(conv10->forward(h9)));
        auto h11 = torch::relu(bn11(conv11->forward(h10)) + h9);
        auto h12 = torch::relu(bn12(conv12->forward(h11)));
        auto h13 = torch::relu(bn13(conv13->forward(h12)) + h11);
        auto h14 = torch::relu(bn14(conv14->forward(h13)));
        auto h15 = torch::relu(bn15(conv15->forward(h14)) + h13);
        auto h16 = torch::relu(bn16(conv16->forward(h15)));

        //policy network

        auto hp = torch::relu(bn_p1(conv_p1->forward(h16)));
        //std::cout << "hp:" << hp.sizes() << std::endl;

        hp = hp.view({ -1, 8 * 8 * 512 });
        //std::cout << "hp:" << hp.sizes() << std::endl;

        auto out_p = fc_p2->forward(hp);
        //std::cout << "output_p:" << out_p.sizes() << std::endl;
        //value
        auto hv = torch::relu(bn_v1(conv_v1->forward(h16)));

        hv = hv.view({ -1, SQUARE_SIZE });
        hv = torch::relu(fc_v2->forward(hv));
        auto out_v = torch::tanh(fc_v3->forward(hv));
        //std::cout << "output_v:" << out_v.sizes() << std::endl;

        return { out_p,out_v };
    }

    torch::nn::Conv2d conv1;
    torch::nn::Conv2d conv2;
    torch::nn::Conv2d conv3;
    torch::nn::Conv2d conv4;
    torch::nn::Conv2d conv5;
    torch::nn::Conv2d conv6;
    torch::nn::Conv2d conv7;
    torch::nn::Conv2d conv8;
    torch::nn::Conv2d conv9;
    torch::nn::Conv2d conv10;
    torch::nn::Conv2d conv11;
    torch::nn::Conv2d conv12;
    torch::nn::Conv2d conv13;
    torch::nn::Conv2d conv14;
    torch::nn::Conv2d conv15;
    torch::nn::Conv2d conv16;

    torch::nn::Conv2d conv_p1;
    torch::nn::Conv2d conv_v1;

    torch::nn::BatchNorm2d bn1;
    torch::nn::BatchNorm2d bn2;
    torch::nn::BatchNorm2d bn3;
    torch::nn::BatchNorm2d bn4;
    torch::nn::BatchNorm2d bn5;
    torch::nn::BatchNorm2d bn6;
    torch::nn::BatchNorm2d bn7;
    torch::nn::BatchNorm2d bn8;
    torch::nn::BatchNorm2d bn9;
    torch::nn::BatchNorm2d bn10;
    torch::nn::BatchNorm2d bn11;
    torch::nn::BatchNorm2d bn12;
    torch::nn::BatchNorm2d bn13;
    torch::nn::BatchNorm2d bn14;
    torch::nn::BatchNorm2d bn15;
    torch::nn::BatchNorm2d bn16;

    torch::nn::BatchNorm2d bn_p1;
    torch::nn::BatchNorm2d bn_v1;

    torch::nn::Linear fc_v2;
    torch::nn::Linear fc_v3;

    torch::nn::Linear fc_p2;
};

TORCH_MODULE(Net);


void make_feat(const Pos &pos, torch::Tensor &feat);
MoveClassPos move_to_index(const Move mv, const Side sd);
void learn();

namespace nn { 
    void test(); 
}

#endif