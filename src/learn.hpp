#ifndef LEARN_HPP
#define LEARN_HPP

#include "game.hpp"
#include <string>

class Learner {
public:
	int thread_id_;
	uint32 file_row_num_;
	std::string file_name_;
	Game game_;
	void phase1();
	void phase2();
};

namespace learner {
	void learn();
}

#endif