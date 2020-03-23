#ifndef LEARN_HPP
#define LEARN_HPP

#include "game.hpp"
#include "learn.hpp"
#include "nn.hpp"
#include <string>
#include <torch/torch.h>

class Learner {
public:
	Learner() {}
	int thread_id_;
	uint32 file_row_num_;
	std::string file_name_;
	Game game_;
	Net *net_;
	torch::Device *device_;
	NNFeat feat_;

	void phase1(torch::optim::Optimizer& optimizer);
	void phase2();
};

namespace learner {
	void learn();
}

#endif