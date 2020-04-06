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
#include <tuple>
#include <torch/torch.h>


std::string SFEN_PATH = "../../all.sfen";
constexpr int MAX_THREAD = 8;
constexpr int BATCH_SIZE_FOR_THREAD = BATCH_SIZE / MAX_THREAD;
std::string MODEL_PATH = "./model.pt";

void Learner::data_load(int batch_size) {
 
    this->feat_.clear();

    for (auto batch_index = 0; batch_index < batch_size; batch_index++) {
        std::ifstream ifs(this->file_name_);
        if (!ifs) {
            Tee << "not found error" << std::endl;
            return;
        }
        std::string line;

        auto row = ml::my_rand(this->file_row_num_ - 1);
        auto num = 0;
        //任意の棋譜へ移動
        while (std::getline(ifs, line)) {
            if ((num++) >= row) {
                break;
            }
        }

        if (std::getline(ifs, line)) {
            std::string arg;
            std::stringstream ss(line);
            this->game_.clear();
            double winner = 0.0;
            while (ss >> arg) {
                if (arg == "startpos" || arg == "moves") {
                    continue;
                }
                Move mv = move::from_usi(arg, this->game_.pos());
                List list;
                gen_legals(list, this->game_.pos());

                if (!list::has_generally(list, mv)) {
                    Tee << this->game_.pos() << std::endl;
                    Tee << move::move_to_string(mv) << std::endl;
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
            auto pos_num = ml::my_rand(this->game_.pos_.size() - 1);

            Pos pos = this->game_.pos(pos_num);
            torch::Tensor f = torch::zeros({ SQUARE_SIZE , POS_END_SIZE, SQUARE_SIZE }, torch::TensorOptions().dtype(torch::kFloat));
            make_feat(pos, f);
            this->feat_.feat_[batch_index] = f;
            const auto mv = this->game_.move_[pos_num];
            this->feat_.policy_target_[batch_index] = move_to_index(mv, pos.turn());
            this->feat_.value_target_[batch_index] = winner;
            this->feat_.pos_list_.add(pos);
            this->feat_.move_list_.add(mv);
        }
        ifs.close();
    }
}

void Learner::valid(torch::Device & device) {
    
    this->data_load(BATCH_SIZE);
    auto feat2 = this->feat_.feat_.view({ BATCH_SIZE, SQUARE_SIZE * POS_END_SIZE ,9,9 });
    
    auto data = feat2.to(device);
    Net model;
    torch::load(model, MODEL_PATH);
    model->to(device);
    model->eval();
    auto output = model->forward(data);
    auto output_p = std::get<0>(output);
    auto output_v = std::get<1>(output);

    output_p = output_p.view({ BATCH_SIZE, CLS_MOVE_END });
    output_v = output_v.view({ BATCH_SIZE });
    auto correct_num = 0.0f;
    auto mse = 0.0f;
    for (auto i = 0; i < BATCH_SIZE; i++) {
        Pos* pos = &this->feat_.pos_list_[i];
        //Tee << *pos << std::endl;
        List list;
        gen_legals(list, this->feat_.pos_list_[i]);
        Move best_move = move::MOVE_NONE;
        auto best_sc = 0.0f;
        for (auto j = 0; j < list.size(); j++) {
            auto index = move_to_index(list[j], pos->turn());
            auto sc = output_p[i][index].item<float>();
            if (sc > best_sc) {
                best_sc = sc;
                best_move = list[j];
            }
        }
        mse += (output_v[i].item<float>() - this->feat_.value_target_[i].item<float>()) 
             * (output_v[i].item<float>() - this->feat_.value_target_[i].item<float>());
        if (best_move == this->feat_.move_list_[i]) {
            correct_num+=1.0f;
        }
        //Tee <<"answer:"<< move::move_to_string(best_move) << std::endl;
        //Tee <<"target:"<< move::move_to_string(this->feat_.move_list_[i]) << std::endl;
        //Tee << "asw:" << output_v[i].item<float>() << std::endl;
        //Tee << "win:" << this->feat_.value_target_[i].item<float>() << std::endl;
    }
    Tee << "percent:" << correct_num / double(BATCH_SIZE) << std::endl;
    Tee << "mse:" << mse / double(BATCH_SIZE) << std::endl;
}

namespace learner {

Learner *gLearner;
static std::thread gThreadList[MAX_THREAD];
    //utilなきがする
    static int row_num(std::string &filename) {
        std::ifstream ifs(filename);
        if (!ifs) {
            Tee << "not found error" << std::endl;
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
        if (/*torch::cuda::is_available()*/false) {
            Tee << "CUDA available! Training on GPU." << std::endl;
            device_type = torch::kCUDA;
        }
        else {
            Tee << "Training on CPU." << std::endl;
            device_type = torch::kCPU;
        }

        torch::Device device(device_type);

        Net model;
        if (ml::is_exists_file(MODEL_PATH)) {
            Tee << "load model\n";
            torch::load(model, MODEL_PATH);
        }
               
        model->to(device);
        model->train();
        torch::optim::SGD optimizer(
            model->parameters(), torch::optim::SGDOptions(0.01));
        gLearner = new Learner[MAX_THREAD];

        for(auto i = 0; i < MAX_THREAD; i++) {
            gLearner[i].thread_id_ = i;
            gLearner[i].file_row_num_ = file_rownum;
            gLearner[i].file_name_ = SFEN_PATH;
        }
        for (auto iterate = 0; iterate < 100000; ++iterate) {
            Tee << "iterate:" << iterate << std::endl;
            optimizer.zero_grad();
            for (auto i = 1; i < MAX_THREAD; i++) {
                gThreadList[i] = std::thread(&Learner::data_load, &gLearner[i], BATCH_SIZE_FOR_THREAD);
            }
            gLearner[0].data_load(BATCH_SIZE_FOR_THREAD);
            for (auto i = 1; i < MAX_THREAD; i++) {
                gThreadList[i].join();
            }
            
            for (auto thread_id = 1; thread_id < MAX_THREAD; thread_id++) {
                for (auto batch_id = 0; batch_id < BATCH_SIZE_FOR_THREAD; batch_id++) {
                    gLearner[0].feat_.feat_[thread_id * BATCH_SIZE_FOR_THREAD + batch_id] =
                        gLearner[thread_id].feat_.feat_[batch_id];
                    gLearner[0].feat_.policy_target_[thread_id * BATCH_SIZE_FOR_THREAD + batch_id] =
                        gLearner[thread_id].feat_.policy_target_[batch_id];
                    gLearner[0].feat_.value_target_[thread_id * BATCH_SIZE_FOR_THREAD + batch_id] =
                        gLearner[thread_id].feat_.value_target_[batch_id];
                }
            }
            auto feat2 = gLearner[0].feat_.feat_.view({ BATCH_SIZE, SQUARE_SIZE * POS_END_SIZE ,9,9 });

            auto data = feat2.to(device);

            auto policy_targets = gLearner[0].feat_.policy_target_.to(device);

            auto value_targets = gLearner[0].feat_.value_target_.to(device);

            //Tee << "data:" << data.sizes() << std::endl;
            auto output = model->forward(data);

            auto output_p = std::get<0>(output);
            auto output_v = std::get<1>(output);

            output_p = output_p.view({ BATCH_SIZE, CLS_MOVE_END });
            output_v = output_v.view({ BATCH_SIZE });

            auto policy_loss_func = torch::nn::CrossEntropyLoss();
            auto value_loss_func = torch::nn::MSELoss();

            //Tee << policy_targets.sizes() << std::endl;
            //Tee << value_targets.sizes() << std::endl;
            //Tee << output_v.sizes() << std::endl;
            //Tee << output_p.sizes() << std::endl;

            //Tee << policy_targets[0] << std::endl;
            //Tee << value_targets[0] << std::endl;
            //Tee << output_v[0] << std::endl;
            //Tee << output_p[0] << std::endl;
            

            auto value_loss = 5 * value_loss_func(output_v, value_targets);
            auto policy_loss = policy_loss_func(output_p, policy_targets);

            auto l2 = 0.0f;
            auto param_vec = model->parameters();
            for (auto &vec : param_vec) {
                auto tmp = torch::pow(vec,2).view(-1);
                l2 += torch::sum(tmp).item<float>();
            }
            l2 *= 0.000001;
            auto l2_penalty = torch::full({1},l2);
            l2_penalty = l2_penalty.to(device);
            auto loss = policy_loss + value_loss + l2_penalty;
            
            AT_ASSERT(!std::isnan(loss.template item<float>()));

            Tee << "loss " << loss.item<float>() << " = " 
                                 << policy_loss.item<float>() << " + " 
                                 << value_loss.item<float>() << " + " 
                                 << l2_penalty.item <float>() << std::endl;

            loss.backward();
            
            optimizer.step();
            
            if (iterate % 100 == 0) {
            	auto no = iterate % 5;
                auto filename = "model" + ml::to_string(no) + ".pt";
                torch::save(model, filename);
                torch::save(model, MODEL_PATH);
                gLearner[0].valid(device);
            }
        }

        delete[] gLearner;
        Tee<<"end learning\n";
    }
}
