#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "attack.hpp"
#include "bit.hpp"
#include "common.hpp"
#include "eval.hpp"
#include "sfen.hpp"
#include "game.hpp"
#include "gen.hpp"
#include "hash.hpp"
#include "libmy.hpp"
#include "list.hpp"
#include "math.hpp"
#include "move.hpp"
#include "pos.hpp"
#include "search.hpp"
#include "sort.hpp"
#include "thread.hpp"
#include "tt.hpp"
#include "util.hpp"
#include "var.hpp"

const std::string EngineName {"jugemu"};
const std::string EngineVersion {"1.0"};

TeeStream Tee;

static void usi_loop();

int main(int /*argc*/, char **/*argv*/) {

    math::init();
    bit::init();
    hash::init();
    pos::init();
    var::init();
    
#ifdef DEBUG
    Tee<<"test start\n";
    //gen::test();
    search::test();
    //attack::test();
    Tee<<"test end\n";
#else
    listen_input();
    var::update();

    usi_loop();
#endif
    return EXIT_SUCCESS;
}

static void usi_loop() {

    Game game;
    game.clear();

    SearchInput si;
    si.init();

    auto init_done = false;

    while(true) {
        std::string line;
        if(!get_line(line)) {
            std::exit(EXIT_SUCCESS);
        }
        if(line.empty()) { continue; }

        std::stringstream ss(line);
        std::string command;
        ss >> command;
        
        Tee<<"from:"<<command<<std::endl;

        if(command == "usi") {
            Tee<<"id name SHOGI"<<std::endl;
            Tee<<"usiok"<<std::endl;

        }  else if (command == "isready") {
            if(!init_done) {
                var::update();
                tt::Gtt.set_size(int64(var::Hash) << (20-4));
                init_done = true;
            }
            Tee << "readyok" << std::endl;
        } else if(command == "setoption") {
            //TODO
        } else if(command == "usinewgame") {
            auto sfen = START_SFEN;
            std::string moves;
            auto parsing_sfen = false;
            auto parsing_moves = false;

            std::string arg;

            while(ss >> arg) {
                if(arg == "startpos") {
                    sfen = START_SFEN;
                    parsing_sfen = false;
                    parsing_moves = false;
                } else if(arg == "sfen") {
                    sfen = "";
                    parsing_sfen = true;
                    parsing_moves = false;
                } else if(arg == "moves") {
                    moves = "";
                    parsing_sfen = false;
                    parsing_moves = true;
                } else if(parsing_sfen) {
                    if(!sfen.empty()) { sfen += " "; }
                    sfen += arg;
                } else if(parsing_moves) {
                    if(!sfen.empty()) { moves += " "; }
                    moves += arg;
                }
            }
            game.init(pos_from_sfen(sfen));
            std::stringstream ss(moves);
            while(ss >> arg) {
                game.add_move(move::from_usi(arg,game.pos()));
            }
            si.init();
        } else if (command == "usinewgame") {
            tt::Gtt.clear();
        } else if(command == "go") {
            //TODO
        } else if(command == "stop") {
            //no-op
        } else if (command == "test") {
           // bit::test();
           // pos::test();
          //  gen::test();
        } else if(command == "quit") {
            std::exit(EXIT_SUCCESS);
        }
    }
}