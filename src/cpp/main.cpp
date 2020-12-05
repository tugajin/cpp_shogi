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
#include "selfplay.hpp"
#include "thread.hpp"
#include "tt.hpp"
#include "util.hpp"
#include "uct.hpp"
#include "var.hpp"
#include "mate_search.hpp"

const std::string EngineName{ "deep_sawa" };
const std::string EngineVersion{ "1.0" };

TeeStream Tee;
Game gGame;

static void usi_loop(std::vector<std::string> arg);

int main(int argc, char** argv) {

	math::init();
	bit::init();
	hash::init();
	pos::init();
	var::init();
	common::init();
	listen_input();
	var::update();


	std::vector<std::string> arg_strings;
	arg_strings.clear();
	if (argc > 1) {
		std::string str = "";
		for (auto i = 0; i < argc - 1; i++) {
			std::string curr = std::string(argv[i + 1]);
			auto pos = curr.find(";");
			if (pos != std::string::npos) {
				str += curr.substr(0, pos);
				arg_strings.push_back(str);
				str = "";
			}
			else {
				if (!str.empty()) { str += " "; }
				str += curr;
			}
		}
		if (!str.empty()) {
			arg_strings.push_back(str);
		}
	}
	usi_loop(arg_strings);
	return EXIT_SUCCESS;
}

static void usi_loop(std::vector<std::string> arg) {

	
	gGame.clear();

	SearchInput si;
	si.init();

	auto init_done = false;

	while (true) {
		std::string line;
		if (!arg.empty()) {
			line = arg.front();
			arg.erase(arg.begin());
		}
		else if (!get_line(line)) {
			std::exit(EXIT_SUCCESS);
		}
		if (line.empty()) { continue; }

		std::stringstream ss(line);
		std::string command;
		ss >> command;

		//Tee<<"from:"<<command<<std::endl;

		if (command == "usi") {
#ifdef DEBUG
			Tee << "id name SAWA_DEBUG" << std::endl;
#else
			Tee << "id name SAWA" << std::endl;
#endif
			Tee << "usiok" << std::endl;

		}
		else if (command == "isready") {
			if (!init_done) {
				var::update();
				gUCT.allocate();
				init_done = true;
			}
			Tee << "readyok" << std::endl;
		}
		else if (command == "setoption") {
			std::string name;
			std::string value;
			auto parsing_name = false;
			auto parsing_value = false;
			std::string arg;
			while (ss >> arg) {
				if (arg == "name") {
					name = "";
					parsing_name = true;
					parsing_value = false;
				}
				else if (arg == "value") {
					value = "";
					parsing_name = false;
					parsing_value = true;
				}
				else if (parsing_name) {
					if (!name.empty()) { name += " "; }
					name += arg;
				}
				else if (parsing_value) {
					if (!value.empty()) { value += " "; }
					value += arg;
				}
			}
			if (false) {
			}
			else {
				var::set(name, value);
				var::update();
			}
		}
		else if (command == "position") {
			auto sfen = START_SFEN;
			std::string moves;
			auto parsing_sfen = false;
			auto parsing_moves = false;

			std::string arg;

			while (ss >> arg) {
				//Tee<<"from2:"<<arg<<std::endl;
				if (arg == "startpos") {
					sfen = START_SFEN;
					parsing_sfen = false;
					parsing_moves = false;
				}
				else if (arg == "sfen") {
					sfen = "";
					parsing_sfen = true;
					parsing_moves = false;
				}
				else if (arg == "moves") {
					moves = "";
					parsing_sfen = false;
					parsing_moves = true;
				}
				else if (parsing_sfen) {
					if (!sfen.empty()) { sfen += " "; }
					sfen += arg;
				}
				else if (parsing_moves) {
					if (!sfen.empty()) { moves += " "; }
					moves += arg;
				}
			}
			gGame.init(pos_from_sfen(sfen));
			std::stringstream ss(moves);
			while (ss >> arg) {
				gGame.add_move(move::from_usi(arg, gGame.pos()));
			}
			si.init();
		}
		else if (command == "usinewgame") {
		}
		else if (command == "go") {
			auto depth = -1;
			auto move_time = -1.0;
			auto smart = false;
			auto moves = 0;
			auto game_time = 30.0;
			auto game_byoyomi = 0.0;
			auto inc = 0.0;
			auto ponder = false;
			auto analyze = false;
			std::string arg;
			while (ss >> arg) {
				//Tee<<"from3:"<<arg<<std::endl;
				if (arg == "depth") {
					ss >> arg;
					depth = std::stoi(arg);
				}
				else if (arg == "movetime") {
					ss >> arg;
					move_time = std::stod(arg) / 1000.0;
				}
				else if (arg == "movestogo") {
					ss >> arg;
					moves = std::stoi(arg);
				}
				else if (arg == (gGame.turn() == BLACK ? "btime" : "wtime")) {
					smart = true;
					ss >> arg;
					game_time = std::stod(arg) / 1000.0;
				}
				else if (arg == (gGame.turn() == BLACK ? "binc" : "winc")) {
					smart = true;
					ss >> arg;
					inc = std::stod(arg) / 1000.0;
				}
				else if (arg == "byoyomi") {
					smart = true;
					ss >> arg;
					game_byoyomi = std::stod(arg) / 1000.0;
				}
				else if (arg == "ponder") {
					ponder = true;
				}
				else if (arg == "infinite") {
					ponder = true;
				}
			}
			if (depth >= 0) { si.depth_ = Depth(depth); }
			if (move_time >= 0.0) { 
				si.type_ = LIMIT_TIME;
				si.time_ = move_time; 
			}
			if (smart) {
				si.set_time(moves, game_time - inc, inc, game_byoyomi);
			} 
			si.move_ = !analyze;
			si.ponder_ = ponder;
			SearchOutput so;
			start_search(so, gGame.pos(), si);
			auto move = so.move_;
			auto answer = so.answer_;
			if (move == move::MOVE_NONE) {
				Tee << "bestmove resign";
			}
			else {
				Tee << "bestmove " << move::move_to_usi(move);
			}/*else if(answer == move::MOVE_NONE) {
				answer = quick_move(game.pos().succ(move));
			}*/

			if (answer != move::MOVE_NONE) {
				Tee << " ponder " << move::move_to_usi(answer);
			}
			Tee << std::endl;
			si.init();
		}
		else if (command == "stop") {
			//no-op
		}
		else if (command == "test") {
			// bit::test();
			// pos::test();
		    //gen::test();
			//search::test_perft();
			//mate::test();
			//uct::test();
			eval::test();
		}
		else if (command == "show") {
			Tee << gGame.pos() << std::endl;
		}
		else if (command == "selfplay") {
			std::string arg;
			auto loop = 0;
			while (ss >> arg) {
				ss >> arg;
				loop = std::stoi(arg);
			}
			gSelfPlay.init();
			for (auto i = 0; i < loop; i++) {
				Tee << "episode:" << i << std::endl;
				gSelfPlay.episode();
				Tee << "end:" << i << std::endl;
			}
			gSelfPlay.free();
			std::exit(EXIT_SUCCESS);			
		} 
		else if (command == "gameover") {
			gUCT.free();
			init_done = false;
		}
		else if (command == "quit") {
			GThread.join();
			std::exit(EXIT_SUCCESS);
		}
	}
}
