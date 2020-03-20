#include "nn.hpp"
#include "pos.hpp"
#include "sfen.hpp"
#include "move.hpp"
#include <torch/torch.h>

#define HIDDEN_NUM 256

struct Net : torch::nn::Module {
    Net()
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
        
        conv_p1(torch::nn::Conv2dOptions(HIDDEN_NUM, CLS_MOVE_END, /*kernel_size=*/2)),
        conv_v1(torch::nn::Conv2dOptions(HIDDEN_NUM, SQUARE_SIZE, /*kernel_size=*/1)),

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

        bn_p1(torch::nn::BatchNorm2dOptions(CLS_MOVE_END)),
        bn_v1(torch::nn::BatchNorm2dOptions(SQUARE_SIZE)),

        fc_v2(SQUARE_SIZE, HIDDEN_NUM),
        fc_v3(HIDDEN_NUM, 1),
        fc_p2(8 * 8 * CLS_MOVE_END, CLS_MOVE_END)
        
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

    torch::Tensor forward(torch::Tensor x) {
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
        auto hp =  torch::relu(bn_p1(conv_p1->forward(h16)));
        std::cout << hp.sizes() << std::endl;
        hp = hp.view({ -1, 8 * 8 * CLS_MOVE_END });
        auto out_p = fc_p2->forward(hp);
        
        //value
        auto hv = torch::relu(bn_v1(conv_v1->forward(h16)));
        hv = hv.view({ -1, SQUARE_SIZE });
        hv = torch::relu(fc_v2->forward(hv));
        auto out_v = torch::tanh(fc_v3->forward(hv));

        return out_p,out_v;
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

void make_feat(const Pos& pos, NNFeat &feat) {
    
    const auto me = pos.turn();
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
                feat.feat_[all_index + index] = torch::ones({ SQUARE_SIZE });
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
                feat.feat_[index][sq] = 1.0;
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
        const auto dir = get_direction(from, to);
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

namespace nn {

    void test() {

        torch::manual_seed(1);

        torch::DeviceType device_type;
        if (torch::cuda::is_available()) {
            std::cout << "CUDA available! Training on GPU." << std::endl;
            device_type = torch::kCUDA;
        }
        else {
            std::cout << "Training on CPU." << std::endl;
            device_type = torch::kCPU;
        }
        torch::Device device(device_type);

        Net model;
        model.to(device);

        torch::optim::SGD optimizer(
            model.parameters(), torch::optim::SGDOptions(0.01).momentum(0.5));

        Pos pos = pos_from_sfen(START_SFEN);
        Tee << pos << std::endl;
        NNFeat f;
        make_feat(pos, f);
        //std::cout << f.feat_ << std::endl;
        Move mv = move::from_usi("7g7f", pos);
        std::cout << move::move_to_string(mv) << std::endl;
        f.policy_target_[move_to_index(mv, pos.turn())] = 1.0;
        f.value_target_[0] = 0.5;
        std::cout << "target end\n";
        for (size_t epoch = 1; epoch <= 10; ++epoch) {
            std::cout << "loop\n";
            model.train();
            std::cout << "loop2\n";
            auto feat2 = f.feat_.view({ 1,POS_END_SIZE ,9,9 });
            std::cout << "size:" << feat2.sizes() << std::endl;
            //auto data = f.feat_.to(device);
            auto data = feat2.to(device);
            auto policy_targets = f.policy_target_.to(device);
            auto value_targets = f.value_target_.to(device);
            std::cout << "loop3\n";
            auto output = model.forward(data);
            
            //auto loss = torch::l1_loss(output, targets);
            auto policy_loss = torch::mse_loss(output, policy_targets);
            auto value_loss = torch::mse_loss(output, value_targets);
            //auto loss = torch::nll_loss(output, targets);
            auto loss = policy_loss + value_loss;
            Tee << "loss:" << loss << std::endl;
            //AT_ASSERT(!std::isnan(loss.template item<float>()));
            Tee << "backword\n";
            loss.backward();
            Tee << "opt\n";
            optimizer.step();
            //test(model, device, *test_loader, test_dataset_size);
        }
        std::cout << "end\n";
    }

}