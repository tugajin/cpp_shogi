#include <fstream>
#include <iostream>
#include <string>
#include "libmy.hpp"
#include "thread.hpp"


class Input : public Waitable {

private:

	std::atomic<bool> has_input_;
	bool eof_;
	std::string line_;

public:

	Input();

	bool peek_line(std::string& line);
	bool get_line(std::string& line);

	void set_eof();
	void set_line(const std::string& line);

	bool has_input() const;
};

static Input GInput;
std::thread GThread;

static void input_program(Input* input) {
	std::string line;
	while (std::getline(std::cin, line)) {
		input->set_line(line);
		if (line == "quit") {
			Tee << "input quit\n";
			break;
		}
	}
	input->set_eof();
}

void listen_input() {
	GThread = std::thread(input_program, &GInput);
}

bool get_line(std::string& line) {
	return GInput.get_line(line);
}

bool has_input() {
	return GInput.has_input();
}

bool peek_line(std::string& line) {
	return GInput.peek_line(line);
}

Input::Input() {
	this->has_input_ = false;
	this->eof_ = false;
}

bool Input::peek_line(std::string& line) {
	this->lock();
	while (!this->has_input_) {
		this->wait();
	}
	auto line_ok = !this->eof_;
	if (line_ok) { line = this->line_; }
	this->unlock();
	return line_ok;
}

bool Input::get_line(std::string& line) {
	this->lock();
	while (!this->has_input_) {
		this->wait();
	}
	auto line_ok = !this->eof_;

	if (line_ok) { line = this->line_; }
	this->has_input_ = false;

	this->signal();
	this->unlock();

	return line_ok;
}

void Input::set_eof() {
	this->lock();
	while (this->has_input_) {
		this->wait();
	}
	this->eof_ = true;
	this->has_input_ = true;
	this->signal();
	this->unlock();
}

void Input::set_line(const std::string& line) {
	this->lock();
	while (this->has_input_) {
		this->wait();
	}
	this->line_ = line;
	this->has_input_ = true;
	this->signal();
	this->unlock();
}

bool Input::has_input() const {
	return this->has_input_;
}

void Lockable::lock() const {
	this->mutex_.lock();
}

void Lockable::unlock() const {
	this->mutex_.unlock();
}

void Waitable::wait() {
	this->cond_.wait(this->mutex_);
}

void Waitable::signal() {
	this->cond_.notify_one();
}