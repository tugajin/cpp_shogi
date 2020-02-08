#ifndef GAME_HPP
#define GAME_HPP

#include "pos.hpp"

class Game {
    public:
    Pos pos;
    void clear();
    void add_move(const Move mv);
    void init(const Pos &pos);
};

#endif