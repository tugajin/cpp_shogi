#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <chrono>
#include "libmy.hpp"

class BadInput : public std::exception {
};

class Timer {

private:

	typedef std::chrono::time_point<std::chrono::system_clock> time_t;
	typedef std::chrono::duration<double> second_t;

	double elapsed_;
	bool running_;
	time_t start_;

public:

	Timer() {
		reset();
	}

	void reset() {
		elapsed_ = 0;
		running_ = false;
	}

	void start() {
		start_ = now();
		running_ = true;
	}

	void stop() {
		elapsed_ += time();
		running_ = false;
	}

	double elapsed() const {
		double time = elapsed_;
		if (running_) time += this->time();
		return time;
	}

private:

	static time_t now() {
		return std::chrono::system_clock::now();
	}

	double time() const {
		assert(running_);
		return std::chrono::duration_cast<second_t>(now() - start_).count();
	}
};

// functions

int find(char c, const std::string& s);


#endif