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
	uint64 num = 0;
	for (auto i = 0; i < list.size(); ++i) {
		auto mv = list[i];
		if (is_root) {
			Tee << i << "/" << list.size() << ":" << move::move_to_string(mv);
		}
#ifdef DEBUG
		if (!move::move_is_ok(mv, pos)) {
			Tee << "move error\n";
			Tee << pos << std::endl;
			Tee << move::move_to_string(mv) << std::endl;
			exit(EXIT_FAILURE);

		}
		//auto pl = move::pseudo_is_legal(mv,pos);
		//auto debug_pl = move::pseudo_is_legal_debug(mv,pos);
#endif
		if (!move::pseudo_is_legal(mv, pos)) {
			continue;
		}
		//Tee<<move::move_to_string(mv)<<" ply:" <<int(ply)<<std::endl;
		auto new_pos = pos.succ(mv);
		const auto n = perft<flip_turn(sd), false>(new_pos, ply - 1);
		if (is_root) {
			Tee << " nodes:" << n << std::endl;
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
	this->smart_ = false;
	this->moves_ = 0;
	this->time_ = 1E6;
	this->inc_ = 0.0;
	this->ponder_ = false;
	this->byoyomi_ = 0.0;
}
void SearchInput::set_time(int moves, double time, double inc, double byoyomi) {
	this->smart_ = true;
	this->moves_ = moves;
	this->time_ = time;
	this->inc_ = inc;
	this->byoyomi_ = byoyomi;
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
}