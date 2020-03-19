#include "nn.hpp"
#include "pos.hpp"
#include "sfen.hpp"
#include "move.hpp"
#include <torch/torch.h>

struct Net : torch::nn::Module {
    Net()
        : conv1(torch::nn::Conv2dOptions(POS_END_SIZE, 192, /*kernel_size=*/3)),
        conv2(torch::nn::Conv2dOptions(192, 300, /*kernel_size=*/3)),
        fc1(300, 500),
        fc2(500, CLS_END){
        register_module("conv1", conv1);
        register_module("conv2", conv2);
        register_module("fc1", fc1);
        register_module("fc2", fc2);
    }

    torch::Tensor forward(torch::Tensor x) {
        x = torch::relu(conv1->forward(x));
        x = torch::relu(conv2->forward(x));
        x = x.view({ -1,300 });
        x = torch::relu(fc1->forward(x));
        x = torch::relu(fc2->forward(x));
        return x;
    }

    torch::nn::Conv2d conv1;
    torch::nn::Conv2d conv2;
    torch::nn::Linear fc1;
    torch::nn::Linear fc2;
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
            /*for(auto all_index = 0; all_index <= num; ++all_index) {
                feat.feat_[all_index] = torch::ones({ SQUARE_SIZE });
            }*/
            feat.feat_[index + num] = torch::ones({ SQUARE_SIZE });
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
        f.target_[move_to_index(mv, pos.turn())] = 1.0;
        f.target_[CLS_VALUE] = 0.5;
        std::cout << "target end\n";
        for (size_t epoch = 1; epoch <= 10; ++epoch) {
            std::cout << "loop\n";
            model.train();
            std::cout << "loop2\n";
            auto feat2 = f.feat_.view({ 1,POS_END_SIZE ,9,9 });
            std::cout << "size:" << feat2.sizes() << std::endl;
            //auto data = f.feat_.to(device);
            auto data = feat2.to(device);
            auto targets = f.target_.to(device);
            std::cout << "loop3\n";
            auto output = model.forward(data);
            std::cout << "out:" <<output[0][CLS_VALUE]<< std::endl;
            
            //auto loss = torch::l1_loss(output, targets);
            auto loss = torch::mse_loss(output, targets);
            //auto loss = torch::nll_loss(output, targets);
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