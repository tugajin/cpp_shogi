
#ifndef LIST_HPP
#define LIST_HPP

#include "common.hpp"
#include "libmy.hpp"
#include "move.hpp"
#include <climits>

class MoveScore {

private:

	int64 pair_;

public:

	MoveScore() {
		pair_ = 0;
	}
	explicit MoveScore(Move mv) : MoveScore(mv, 0) {
	}
	MoveScore(Move mv, int sc) {
		assert(mv != move::MOVE_NONE);
		assert(uint64(mv) < (uint64(1) << 31));
		assert(uint64(std::abs(sc)) < (uint64(1) << 31));
		pair_ = (int64(sc) << 32) | int(mv);
	}

	friend bool operator < (MoveScore m0, MoveScore m1) {
		return m0.pair_ < m1.pair_;
	}

	void set_score(int sc) {
		assert(uint64(std::abs(sc)) < (uint64(1) << 31));
		pair_ = (int64(sc) << 32) | uint32(pair_);
	}

	Move move() const {
		return Move(uint32(pair_));
	}
	int  score() const {
		return pair_ >> 32;
	}
};


namespace list {

	inline int  find(const List& list, Move mv);

	inline bool has(const List& list, Move mv);

}

class List {

private:

	static constexpr int Size = 600;

	ml::Array<MoveScore, Size> pair_;

public:

	void clear() {
		pair_.clear();
	}

	void add(Move mv) {
		assert(!(list::has(*this, mv)));
		pair_.add(MoveScore(mv));
	}
	void add(Move mv, int sc) {
		assert(!(list::has(*this, mv)));
		pair_.add(MoveScore(mv, sc));
	}

	void set_size(int size) {
		assert(size <= this->size());
		pair_.set_size(size);
	}
	void set_score(int i, int sc) {
		assert(i >= 0 && i < size());
		pair_[i].set_score(sc);
	}

	void move_to_front(int i) {
		assert(i >= 0 && i < size());
		// stable "swap"
		MoveScore pair = pair_[i];

		for (int j = i; j > 0; j--) {
			pair_[j] = pair_[j - 1];
		}

		pair_[0] = pair;
	}
	void sort() {
		// init
		int size = this->size();
		if (size <= 1) return;

		// insert sort (stable)

		//pair_.add(MoveScore(move::MOVE_NULL, -((uint64(1) << 31) - 1))); // HACK: sentinel
		pair_.add(MoveScore(move::MOVE_NULL, -32768));

		for (int i = size - 2; i >= 0; i--) {

			auto pair = pair_[i];

			int j;

			for (j = i; pair < pair_[j + 1]; j++) {
				pair_[j] = pair_[j + 1];
			}

			assert(j < size);
			pair_[j] = pair;
		}

		pair_.remove(); // sentinel
	}

	int size() const {
		return pair_.size();
	}
	Move move(int i) const {
		assert(i >= 0 && i < size());
		return pair_[i].move();
	}
	int  score(int i) const {
		assert(i >= 0 && i < size());
		return pair_[i].score();
	}

	Move operator [] (int i) const {
		return move(i);
	}

	friend std::ostream& operator<<(std::ostream& os, const List& list) {
		for (auto i = 0; i < list.size(); i++) {
			os << std::setw(3) << i << ":" << move::move_to_string(list.move(i)) << std::endl;
		}
		return os;
	}

};

namespace list {

	inline int find(const List& list, Move mv) {
		for (auto i = 0; i < list.size(); ++i) {
			if (list[i] == mv) return i;
		}

		return -1;
	}

	inline bool has(const List& list, Move mv) {
		return find(list, mv) >= 0;
	}

	inline int find_generally(const List& list, Move mv) {
		for (auto i = 0; i < list.size(); ++i) {
			const auto list_mv = list[i];
			const auto from1 = move::move_from(list_mv);
			const auto to1 = move::move_to(list_mv);
			const auto piece1 = move::move_piece(list_mv);
			const auto cap1 = move::move_cap(list_mv);

			const auto from2 = move::move_from(mv);
			const auto to2 = move::move_to(mv);
			const auto piece2 = move::move_piece(mv);
			const auto cap2 = move::move_cap(mv);
			if (from1 == from2 &&
				to1 == to2 &&
				piece1 == piece2 &&
				cap1 == cap2) {
				return i;
			}
		}

		return -1;
	}

	inline bool has_generally(const List& list, Move mv) {
		return find_generally(list, mv) >= 0;
	}


}


#endif //
