#ifndef SELFPLAY_HPP
#define SELFPLAY_HPP

#include "pos.hpp"
#include "uct.hpp"
#include "search.hpp"
#include <vector>
#include <string>

class SelfPlay {
public:
	UCTSearcher player_[2];
	SearchInput player_si_[2];
	SearchOutput player_so_[2];

	std::vector<std::string> info_;

	void init();
	void free();
	void episode();
	void out(int result) const ;
};

extern SelfPlay gSelfPlay;

#endif
