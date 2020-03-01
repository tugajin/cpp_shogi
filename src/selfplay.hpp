#ifndef SELFPLAY_HPP
#define SELFPLAY_HPP

#include "uct.hpp"

class SelfPlay {
public:
	UCTSearcher player1_;
	UCTSearcher player2_;
};

#endif
