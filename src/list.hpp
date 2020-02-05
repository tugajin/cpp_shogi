
#ifndef LIST_HPP
#define LIST_HPP

#include "common.hpp"
#include "libmy.hpp"

class MoveScore {

private :

   int pair_;

public :

    MoveScore () {
        pair_ = 0;
    }
    explicit MoveScore (Move mv) : MoveScore(mv, 0) {   
    }
    MoveScore (Move mv, int sc) {
        assert(mv != move::None);
        assert(int(mv) >= 0 && int(mv) < (1 << 15));
        assert(std::abs(sc) < (1 << 15));
        pair_ = (sc << 16) | int(mv);
    }

    friend bool operator < (MoveScore m0, MoveScore m1) {
        return m0.pair_ < m1.pair_;
    }

    void set_score (int sc) {
        assert(std::abs(sc) < (1 << 15));
        pair_ = (sc << 16) | uint16(pair_);
    }

    Move move  () const {
        return Move(uint16(pair_));
    }
    int  score () const {
        return pair_ >> 16;
    }
};

class List {

private :

   static const int Size = 600;

   ml::Array<MoveScore, Size> pair_;

public :

    void clear();

    void add (Move mv) {
        assert(!(list::has(*this, mv)));
        pair_.add(MoveScore(mv));
   }
   void add (Move mv, int sc) {
        assert(!(list::has(*this, mv)));
        pair_.add(MoveScore(mv, sc));
   }

   void set_size  (int size) {
        assert(size <= this->size());
        pair_.set_size(size);
   }
   void set_score (int i, int sc) {
        assert(i >= 0 && i < size());
        pair_[i].set_score(sc);
   }

    void move_to_front  (int i) {
       assert(i >= 0 && i < size());
       // stable "swap"
        MoveScore pair = pair_[i];

        for (int j = i; j > 0; j--) {
            pair_[j] = pair_[j - 1];
        }

        pair_[0] = pair;
   }
   void sort () {
        // init
        int size = this->size();
        if (size <= 1) return;

        // insert sort (stable)

        pair_.add(MoveScore(move::MOVE_NULL, -((1 << 15) - 1))); // HACK: sentinel

        for (int i = size - 2; i >= 0; i--) {

            MoveScore pair = pair_[i];

            int j;

            for (j = i; pair < pair_[j + 1]; j++) {
                pair_[j] = pair_[j + 1];
            }

            assert(j < size);
            pair_[j] = pair;
        }

        pair_.remove(); // sentinel
   }

   int size  () const {
        return pair_.size();
   }
   Move move  (int i) const {
          assert(i >= 0 && i < size());
   return pair_[i].move();
   }
   int  score (int i) const {
        assert(i >= 0 && i < size());
        return pair_[i].score();
   }

   Move operator [] (int i) const {
       return move(i);
   }
};

namespace list {

inline int  find (const List & list, Move mv){
  for (int i = 0; i < list.size(); i++) {
      if (list[i] == mv) return i;
   }

   return -1;
}

inline bool has (const List & list, Move mv) {
    return find(list, mv) >= 0;
}
}
#endif //
