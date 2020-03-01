#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "common.hpp"
#include "libmy.hpp"
#include "pos.hpp"
#include "score.hpp"
#include "util.hpp"
#include "move.hpp"

class List;
class Pos;

const Depth DEPTH_MAX = Depth(128);
const Ply PLY_ROOT = Ply(0);
const Ply PLY_MAX = Ply(127);
const int PLY_SIZE = PLY_MAX + 1;
constexpr int MAX_LEGAL_MOVES = 600;

class Line {
private:
	ml::Array<Move, PLY_SIZE> move_;
public:
	Line() {
		this->clear();
	}
	void clear() {
		move_.clear();
	}
	void add(const Move mv) {
		assert(move::move_is_ok(mv));
		move_.add(mv);
	}
	void set(const Move mv) {
		clear();
		add(mv);
	}
	void concat(const Move mv, const Line& pv) {
		clear();
		add(mv);
		for (auto i = 0; i < pv.size(); ++i) {
			add(pv[i]);
		}
	}
	int size() const {
		return move_.size();
	}
	Move move(const int i) const {
		return move_[i];
	}
	Move operator[](const int i) const {
		return move(i);
	}
	std::string to_usi() const {
		std::string s = "";
		for (auto i = 0; i < this->size(); i++) {
			if (!s.empty()) { s += " "; }
			s += move::move_to_usi(move(i));
		}
		return s;
	}
};
class SearchInput {
public:
	bool move_;
	Depth depth_;
	bool smart_;
	int moves_;
	double time_;
	double inc_;
	double byoyomi_;
	bool ponder_;
	void init();
	void set_time(int moves, double time, double inc, double byoyomi);
};

class SearchOutput {
public:
	Move move_;
	Move answer_;
	Score score_;
	Depth depth_;
	Line pv_;
	int64 node_;
	int ply_max_;

private:
	const SearchInput* si_;
	Pos pos_;
	mutable Timer timer_;

public:
	void init(const SearchInput& si, const Pos& pos);
	void end();
	void start_iter(Depth depth);
	void end_iter();
	void new_best_move(Move move, Score sc);
	void new_best_move(Move mv, Score sc, const Line& pv);
	void disp_best_move();
	double time() const;
};

Move quick_move(const Pos& pos);
Score quick_score(const Pos& pos);


namespace search {
	void test();
}
#endif