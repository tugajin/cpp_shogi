#include "selfplay.hpp"
#include "pos.hpp"
#include "sfen.hpp"
#include "search.hpp"
#include "game.hpp"
#include "gen.hpp"
#include "list.hpp"
#include <fstream>

SelfPlay gSelfPlay;

void SelfPlay::init() {
	this->info_.clear();
	for (auto i = 0; i < 2; i++) {
		this->player_[i].allocate();
	}
}
void SelfPlay::free() {
	for (auto i = 0; i < 2; i++) {
		this->player_[i].free();
	}
}
void SelfPlay::episode() {
	gGame.clear();
	gGame.init(pos_from_sfen(START_SFEN));
	//gGame.init(pos_from_sfen("lns2k3/3g5/ppppppp2/3ll4/9/7P1/P1N2PP2/5G3/2G1K1+p b B6pl2n3sb2rg"));
	auto tesu = 0u;
	auto result = 2;//0:先手が勝ち 1:後手が勝ち 2:引き分け
	const auto rand_tesu = ml::rand_int_64() % 150;
	//const int rand_tesu = 0;

	while (true) {
		Tee << gGame.pos() << std::endl;
		const auto turn = gGame.pos().turn();
		
		List list;
		Tee << "before\n";
		gen_legals(list, gGame.pos());
		Tee << "end\n";
		Tee <<"size:"<< list.size() << std::endl;
		//Tee << list << std::endl;

		if (list.size() == 0) {
			Tee << "aa\n";
			//保存しない
			if (tesu < rand_tesu) {
				Tee << "bb\n";
				return;
			}
			if (turn == BLACK) {
				result = 1;
			}
			else {
				result = 0;
			}
			Tee << "cc\n";
			break;
		}

		if ((tesu++) > 256) {
			result = 2;
			break;
		}
		if (tesu < rand_tesu) {
			const auto index = ml::rand_int_64() % list.size();
			const auto mv = list[index];
			gGame.add_move(mv);
			continue;
		}
		
		this->player_[turn].pos_ = gGame.pos();
		this->player_si_[turn].init();
		this->player_si_[turn].node_ = 5000;
		this->player_si_[turn].type_ = LIMIT_NODE;
		this->player_so_[turn].init(this->player_si_[turn], gGame.pos());

		this->player_[turn].so_ = &this->player_so_[turn];
		this->player_[turn].think();
		if (this->player_so_[turn].move_ == move::MOVE_NONE) {
			if (turn == BLACK) {
				result = 1;
			}
			else {
				result = 0;
			}
			break;
		} 
		else {
			auto bb = gGame.pos().pieces(turn);
			auto num = bb.pop_cnt();
			HAND_FOREACH(p) {
				num += hand_num(gGame.pos().hand(turn), p);
			}
			if (num < 5) {//負けでいいでしょう。
				Tee << "特別ルール\n";
				if (turn == BLACK) {
					result = 1;
				}
				else {
					result = 0;
				}
				break;
			}
		}
		Tee << move::move_to_string(this->player_so_[turn].move_) << std::endl;
		this->info_.push_back("state;" + out_sfen(gGame.pos()));
		this->info_.push_back("action;" + this->player_[turn].out_root_info());
		gGame.add_move(this->player_so_[turn].move_);
	}
	this->out(result);
}
void SelfPlay::out(int result) const {
	Tee << "dd\n";
	std::ofstream out("selfplay.sfen", std::ios::app);
	Tee << "ee\n";
	for (auto v : this->info_) {
		out <<result<<";"<< v << std::endl;
	}
	Tee << "ff\n";
}