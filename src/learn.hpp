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
	NNFeat feat_;

	void data_load(int batch_size);
	void valid(torch::Device& device);
};

namespace learner {
	void learn();
}

#endif