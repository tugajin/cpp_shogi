#include "learn.hpp"
#include "game.hpp"
#include "pos.hpp"
#include "gen.hpp"
#include "list.hpp"
#include "libmy.hpp"
#include "nn.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <torch/torch.h>


std::string SFEN_PATH = "C:/Users/tugajin/Documents/rsc/all.sfen";

void Learner::phase1() {

    std::ifstream ifs(this->file_name_);
    if (!ifs) {
        std::cout << "not found error" << std::endl;
        return;
    }
    std::string line;
    
    auto row = ml::my_rand(this->file_row_num_);
    auto num = 0;
    //任意の棋譜へ移動
    //クソダサい
    while(std::getline(ifs,line)) {
        if((num++) >= row) {
            break;
        }
    }

    if (std::getline(ifs, line)) {
        //std::cout << line << std::endl;
        std::string arg;
        std::stringstream ss(line);
        this->game_.clear();
        double winner = 0.0;
        while (ss >> arg) {
            if (arg == "startpos" || arg == "moves") {
                continue;
            }
            //std::cout << arg << std::endl;
            Move mv = move::from_usi(arg, this->game_.pos());
            //Tee<<move::move_to_string(mv)<<std::endl;
            List list;
            gen_legals(list, this->game_.pos());
            if (!list::has(list, mv)) {
                //Tee<<"thread " <<this->thread_id_<<std::endl;
                //std::cout << this->game_.pos() << std::endl;
                //std::cout << move::move_to_string(mv) << std::endl;
            }
            if (!list::has_generally(list,mv)) {
                std::cout << this->game_.pos() << std::endl;
                std::cout << move::move_to_string(mv) << std::endl;
                exit(1);
            }
            this->game_.add_move(mv);
        }
        if (this->game_.pos_.size() % 2 == 0) {
            winner = 1.0;
        }
        else {
            winner = -1.0;
        }
        auto pos_num = ml::my_rand(this->game_.pos_.size());
        Pos pos = this->game_.pos(pos_num);
        this->feat_.clear();
        make_feat(pos, this->feat_);
        auto feat2 = this->feat_.feat_.view({ 1,POS_END_SIZE ,9,9 });
        auto data = feat2.to(*this->device_);
        auto policy_targets = this->feat_.policy_target_.to(*this->device_);
        auto value_targets = this->feat_.value_target_.to(*this->device_);
        auto output = this->net_->forward(data);
        auto policy_loss = torch::mse_loss(output, policy_targets);
        auto value_loss = torch::mse_loss(output, value_targets);
        auto loss = policy_loss + value_loss;
        std::cout << "loss:" << loss << std::endl;
    }

}

void Learner::phase2() {

}

namespace learner {

constexpr int MAX_THREAD = 1;
Learner *gLearner;
static std::thread gThreadList[MAX_THREAD];
    //utilなきがする
    static int row_num(std::string &filename) {
        std::ifstream ifs(filename);
        if (!ifs) {
            std::cout << "not found error" << std::endl;
            return -1;
        }
        auto num = 0u;
        std::string line;
        while(getline(ifs,line)) { ++num; }
        return num;
    }
    void learn() {
        Tee<<"start learning\n";
        auto file_rownum = row_num(SFEN_PATH);
        if(file_rownum < 0) {
            Tee<<"file not found\n";
            return;
        }
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
        model.train();
        gLearner = new Learner[MAX_THREAD];

        for(auto i = 0; i < MAX_THREAD; i++) {
            gLearner[i].thread_id_ = i;
            gLearner[i].file_row_num_ = file_rownum;
            gLearner[i].file_name_ = SFEN_PATH;
            gLearner[i].net_ = &model;
            gLearner[i].device_ = &device;
        }
        for (auto epoch = 0; epoch < 10; epoch++) {
            std::cout << "epoch:" << epoch << std::endl;
            for (auto loop = 0; loop < 10; loop++) {
                for (auto i = 1; i < MAX_THREAD; i++) {
                    gThreadList[i] = std::thread(&Learner::phase1, gLearner[i]);
                }
                std::cout << "phase1\n";
                gLearner[0].phase1();
                for (auto i = 1; i < MAX_THREAD; i++) {
                    gThreadList[i].join();
                }
            }
            //phase2
            std::cout << "phase2\n";
            torch::optim::SGD optimizer(
                model.parameters(), torch::optim::SGDOptions(0.01).momentum(0.5));
        }

        delete[] gLearner;
        Tee<<"end learning\n";
    }
}
