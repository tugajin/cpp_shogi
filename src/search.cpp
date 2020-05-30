#include "search.hpp"
#include "attack.hpp"
#include "gen.hpp"
#include "move.hpp"
#include "sfen.hpp"
#include "list.hpp"
#include "uct.hpp"
#include "var.hpp"

template<Side sd, bool is_root> static uint64 perft(const Pos& pos, const Ply ply) {
#ifdef DEBUG
	if (!pos.is_ok()) {
		Tee << "error\n";
		Tee << pos << std::endl;
		Tee << move::move_to_string(pos.last_move()) << std::endl;
		exit(EXIT_FAILURE);
	}
	if (!is_legal(pos)) {
		Tee << "legal error\n";
		Tee << pos << std::endl;
		Tee << move::move_to_string(pos.last_move()) << std::endl;
		exit(EXIT_FAILURE);
	}
#endif

	if (ply <= 0) {
		return 1;
	}
	List list;
	gen_moves<sd>(list, pos);
	
	List check_list;
	gen_moves<CHECK,sd>(check_list,pos,nullptr);

	uint64 num = 0;
	for (auto i = 0; i < list.size(); ++i) {
		auto mv = list[i];
		if (is_root) {
			//Tee << i << "/" << list.size() << ":" << move::move_to_string(mv);
		}
#ifdef DEBUG
		if (!move::move_is_ok(mv, pos)) {
			Tee << "move error\n";
			Tee << pos << std::endl;
			Tee << move::move_to_string(mv) << std::endl;
			exit(EXIT_FAILURE);

		}
		const auto pseudo = move::pseudo_is_legal(mv,pos);
		const auto pseudo_debug = move::pseudo_is_legal_debug(mv,pos);
		if(pseudo != pseudo_debug) {
			Tee<<"pseudo not eq\n";
			Tee<<pos<<std::endl;
			Tee<<move::move_to_string(mv)<<std::endl;
			exit(EXIT_FAILURE);
		}
#endif

		if (!move::pseudo_is_legal(mv, pos)) {
			continue;
		}
		const auto check = move::is_check(mv,pos);
#ifdef DEBUG
		if(check) {
			if(!list::has(check_list,mv)) {
				Tee<<"check move erro1\n";
				Tee<<pos<<std::endl;
				Tee<<move::move_to_string(mv)<<std::endl;
				Tee<<"\n";
				Tee<<check_list<<std::endl;
				exit(EXIT_FAILURE);
			}
		} else {
			if(list::has(check_list,mv)) {
				Tee<<"check move erro2\n";
				Tee<<pos<<std::endl;
				Tee<<move::move_to_string(mv)<<std::endl;
				Tee<<"\n";
				Tee<<check_list<<std::endl;
				exit(EXIT_FAILURE);
			}
		}
#endif

		auto new_pos = pos.succ(mv);
#ifdef DEBUG

		const auto check_debug = in_check(new_pos);
		if(check != check_debug) {
			Tee<<"check not eq\n";
			Tee<<pos<<std::endl;
			Tee<<move::move_to_string(mv)<<std::endl;
			Tee<<check<<std::endl;
			Tee<<check_debug<<std::endl;
			exit(EXIT_FAILURE);
		}
#endif
		const auto n = perft<flip_turn(sd), false>(new_pos, ply - 1);
		if (is_root) {
			//Tee << " nodes:" << n << std::endl;
		}

		num += n;
	}
	return num;
}

uint64 perft(const Pos& pos, Ply ply) {
	return (pos.turn() == BLACK) ? perft<BLACK, true>(pos, ply)
		: perft<WHITE, true>(pos, ply);
}
void SearchInput::init() {
	var::update();
	this->move_ = true;
	this->depth_ = Depth(128);
	this->moves_ = 0;
	this->time_ = 1E6;
	this->inc_ = 0.0;
	this->ponder_ = false;
	this->byoyomi_ = 0.0;
	this->type_ = LIMIT_NODE;
}
void SearchInput::set_time(int moves, double time, double inc, double byoyomi) {
	this->moves_ = moves;
	this->time_ = time;
	this->inc_ = inc;
	this->byoyomi_ = byoyomi;
	this->type_ = LIMIT_SMART_TIME;
}
void SearchInput::set_node(uint64 node) {
	this->node_ = node;
	this->type_ = LIMIT_NODE;
}
void SearchOutput::init(const SearchInput& si, const Pos& pos) {
	this->si_ = &si;
	this->pos_ = pos;
	this->move_ = move::MOVE_NONE;
	this->answer_ = move::MOVE_NONE;
	this->score_ = Score(0);
	this->depth_ = Depth(0);
	this->pv_.clear();
	this->timer_.reset();
	this->timer_.start();
	this->node_ = 0;
	this->ply_max_ = 0;
}
void SearchOutput::end() {
	this->timer_.stop();
}
double SearchOutput::time() const {
	return this->timer_.elapsed();
}
template uint64 perft<BLACK, true>(const Pos& pos, const Ply ply);
template uint64 perft<WHITE, true>(const Pos& pos, const Ply ply);
template uint64 perft<BLACK, false>(const Pos& pos, const Ply ply);
template uint64 perft<WHITE, false>(const Pos& pos, const Ply ply);

namespace search {
	void test() {
		//Pos pos = pos_from_sfen(START_SFEN);
		Pos pos = pos_from_sfen("l6nl/5+P1gk/2np1S3/p1p4Pp/3P2Sp1/1PPb2P1P/P5GS1/R8/LN4bKL w GR5pnsg 1");
		Tee << pos << std::endl;
		Timer t;
		t.start();
		auto num = perft(pos, Ply(5));
		t.stop();
		std::cout << num << std::endl;
		std::cout << t.elapsed() << std::endl;
	}

	void test_perft() {
		for(auto i = 0; i < 100000; i++) {
			Pos pos = pos_from_sfen(START_SFEN);
			for(auto j = 0; j < 512; j++) {
				List ml;
				Tee<<pos<<std::endl;
				gen_legals(ml,pos);
				const auto num = perft(pos,Ply(3));
				std::cout<<"num:"<<num<<std::endl;
				if (ml.size() == 0) {
					break;
				}
				const auto index = ml::my_rand(ml.size());
				const auto mv = ml.move(index);
				pos = pos.succ(mv);
			}
		}
	}
}