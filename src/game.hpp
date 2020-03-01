#ifndef GAME_HPP
#define GAME_HPP

#include "pos.hpp"
#include "move.hpp"

class Game {
private:
	static constexpr int Size = 4096;
public:
	Pos pos_start_;
	ml::Array<Pos, Size> pos_;
	ml::Array<Move, Size> move_;
	Game() {
		clear();
	}
	void clear() {
		init(pos::gStart);
	}
	void init(const Pos& pos) {
		this->pos_start_ = pos;
		this->move_.clear();
		this->pos_.clear();
		this->pos_.add_ref(pos);
	}
	void add_move(const Move mv) {
		assert(mv != move::MOVE_NONE);
		this->move_.add(mv);
		this->pos_.add_ref(this->pos().succ(mv));
	}
	Side turn() const {
		return pos().turn();
	}
	const Pos& pos() const {
		assert(this->pos_.size() > 0);
		return this->pos_[this->pos_.size() - 1];
	}
};

extern Game gGame;

#endif