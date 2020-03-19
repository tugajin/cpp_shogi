#include "learn.hpp"
#include "game.hpp"
#include "pos.hpp"
#include "gen.hpp"
#include "list.hpp"
#include "libmy.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <thread>

std::string SFEN_PATH = "/home/tugajin/Documents/etc/resource/wdoor2019/original.sfen";

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
        auto pos_num = ml::my_rand(this->game_.pos_.size());
        Pos pos = this->game_.pos(pos_num);
        Tee<<"thread "<<this->thread_id_<<std::endl;
        Tee<<pos<<std::endl;
    }

}

void Learner::phase2() {

}

namespace learner {

constexpr int MAX_THREAD = 6;
static Learner gLearner[MAX_THREAD];
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
        for(auto i = 0; i < MAX_THREAD; i++) {
            gLearner[i] = Learner();
            gLearner[i].thread_id_ = i;
            gLearner[i].file_row_num_ = file_rownum;
            gLearner[i].file_name_ = SFEN_PATH;
        }
        for(auto loop = 0; loop < 10; loop++) {
            for(auto i = 0; i < MAX_THREAD; i++) {
                gThreadList[i] = std::thread(&Learner::phase1,gLearner[i]);
            }
            for(auto &th : gThreadList) {
                th.join();
            }
        }
        Tee<<"end learning\n";
    }
}
