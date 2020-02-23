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
#include "uct.hpp"
#include "var.hpp"

const std::string EngineName {"jugemu"};
const std::string EngineVersion {"1.0"};

TeeStream Tee;

static void usi_loop(std::vector<std::string> arg);

int main(int argc, char **argv) {

    math::init();
    bit::init();
    hash::init();
    pos::init();
    var::init();
    
//#ifdef DEBUG
 //   Tee<<"test start\n";
    //gen::test();
    //search::test();
    //attack::test();
    //uct::test();
    //pos::test();
  //  Tee<<"test end\n";
//#else
    listen_input();
    var::update();

    std::vector<std::string> arg_strings;
    arg_strings.clear();
    if (argc  > 1) {
        for(auto i = 0; i < argc-1; i++) {
            //逆向きに入れる
            arg_strings.push_back(argv[argc-i-1]);
        }
    }

    usi_loop(arg_strings);
//#endif
    return EXIT_SUCCESS;
}

static void usi_loop(std::vector<std::string> arg) {

    Game game;
    game.clear();

    SearchInput si;
    si.init();

    auto init_done = false;

    while(true) {
        std::string line;
        if(!arg.empty()) {
            line = arg.back();
            arg.pop_back();
        } else if(!get_line(line)) {
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
                //tt::Gtt.set_size(int64(var::Hash) << (20-4));
                gUCT.allocate();
                init_done = true;
            }
            Tee << "readyok" << std::endl;
        } else if(command == "setoption") {
            std::string name;
            std::string value;
            auto parsing_name = false;
            auto parsing_value = false;
            std::string arg;
            while(ss >> arg) {
                if(arg == "name") {
                    name = "";
                    parsing_name = true;
                    parsing_value = false;
                } else if(arg == "value") {
                    value = "";
                    parsing_name = false;
                    parsing_value = true;
                } else if(parsing_name) {
                    if(!name.empty()) { name += " ";}
                    name += arg;
                } else if(parsing_value) {
                    if(!value.empty()) { value += " ";}
                    value += arg;
                }
            }
            if(false) {
            } else {
                var::set(name,value);
                var::update();
            }
        } else if(command == "position") {
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
                Tee<<arg<<std::endl;
                game.add_move(move::from_usi(arg,game.pos()));
            }
            si.init();
        } else if (command == "usinewgame") {
        } else if(command == "go") {
            auto depth = -1;
            auto move_time = -1.0;
            auto smart = false;
            auto moves = 0;
            auto game_time = 30.0;
            auto inc = 0.0;
            auto ponder = false;
            auto analyze = false;
            std::string arg;
            while(ss >> arg) {
                if(arg == "depth") {
                    ss >> arg;
                    depth = std::stoi(arg);
                } else if(arg == "movetime") {
                    ss >> arg;
                    move_time = std::stod(arg) / 1000.0;
                } else if(arg == "movestogo") {
                    ss >> arg;
                    moves = std::stoi(arg);
                } else if(arg == (game.turn() == BLACK? "btime" : "wtime")) {
                    smart = true;
                    ss >> arg;
                    game_time = std::stod(arg) / 1000.0;
                } else if(arg == (game.turn() == BLACK? "binc" : "winc")) {
                    smart = true;
                    ss >> arg;
                    inc = std::stod(arg) / 1000.0;
                } else if(arg == "ponder") {
                    ponder = true;
                } else if(arg == "infinite") {
                    ponder = true;
                }
            }
            if(depth >= 0) { si.depth_ = Depth(depth); }
            if(move_time >= 0.0) { si.time_ = move_time; }
            if(smart) {
                si.set_time(moves,game_time - inc,inc);
            }
            si.move_ = !analyze;
            si.ponder_ = ponder;
            SearchOutput so;
            start_search(so,game.pos(),si);
            auto move = so.move_;
            auto answer = so.answer_;
            /*if(move == move::MOVE_NONE) {
                move = quick_move(game.pos());
            } else if(answer == move::MOVE_NONE) {
                answer = quick_move(game.pos().succ(move));
            }*/
            Tee<<"bestmove "<<move::move_to_usi(move);
            if(answer != move::MOVE_NONE) {
                Tee<<" ponder "<<move::move_to_usi(answer);
            }
            Tee<<std::endl;
            si.init();
        } else if(command == "stop") {
            //no-op
        } else if (command == "test") {
           // bit::test();
           // pos::test();
          //  gen::test();
        } else if(command == "show") {
            Tee<<game.pos()<<std::endl;
        } else if(command == "quit") {
            GThread.join();
            std::exit(EXIT_SUCCESS);
        }
    }
}