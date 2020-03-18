#ifndef LEARN_HPP
#define LEARN_HPP


class Learner {
public:
	Learner(int id) {
		this->thread_id_ = id;
	}
	int thread_id_;
	void phase1();
	void phase2();
};

namespace learner {
	void learn();
}

#endif