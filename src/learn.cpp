#include "learn.hpp"
#include "game.hpp"
#include "pos.hpp"
#include "gen.hpp"
#include "list.hpp"
#include <iostream>
#include <fstream>
#include <string>

void Learner::phase1() {

    std::ifstream ifs("C:/Users/tugajin/Documents/rsc/all.sfen");
    if (!ifs) {
        std::cout << "ファイルが開けませんでした。" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(ifs, line)) {
        //std::cout << line << std::endl;
        std::string arg;
        std::stringstream ss(line);
        gGame.clear();
        while (ss >> arg) {
            if (arg == "startpos" || arg == "moves") {
                continue;
            }
            //std::cout << arg << std::endl;
            Move mv = move::from_usi(arg, gGame.pos());
            List list;
            gen_legals(list, gGame.pos());
            if (!list::has(list, mv)) {
                std::cout << gGame.pos() << std::endl;
                std::cout << move::move_to_string(mv) << std::endl;
            }
            if (!list::has_generally(list,mv)) {
                std::cout << gGame.pos() << std::endl;
                std::cout << move::move_to_string(mv) << std::endl;
                exit(1);
            }
            gGame.add_move(mv);
        }
    }

}

void Learner::phase2() {

}

namespace learner {

    void learn() {
        Learner l = Learner(0);
        l.phase1();
    }

}
