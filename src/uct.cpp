#include "uct.hpp"
#include "pos.hpp"
#include "move.hpp"
#include "common.hpp"
#include "gen.hpp"
#include "list.hpp"
#include "score.hpp"
#include "search.hpp"
#include "sfen.hpp"
#include "eval.hpp"
#include "thread.hpp"
#include "nn.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <torch/torch.h>

UCTSearcher gUCT;

void UCTSearcher::allocate() {
	this->uct_nodes_size_ = (uint32(1) << 14);
	if (this->uct_nodes_ != nullptr) {
		this->free();
	}
	this->uct_nodes_ = new UCTNode[this->uct_nodes_size_];
	assert(this->uct_nodes_ != nullptr);

    if (torch::cuda::is_available()) {
        Tee << "CUDA available! Training on GPU." << std::endl;
        this->device_type_ = torch::kCUDA;
    }
    else {
        Tee << "Training on CPU." << std::endl;
        this->device_type_ = torch::kCPU;
    }

    torch::Device device(this->device_type_);
	Tee<<"load model...\n";
	torch::load(this->model_, "../model.pt");
	this->model_->to(device);
	this->model_->eval();
	Tee<<"end\n";
}
void UCTSearcher::free() {
	delete[] this->uct_nodes_;
	this->uct_nodes_ = nullptr;
}
void UCTSearcher::init() {
	this->uct_nondes_mask_ = this->uct_nodes_size_ - 1;
	this->use_node_num_ = 0;
	for (auto i = 0u; i < this->uct_nodes_size_; ++i) {
		this->uct_nodes_[i].clear();
	}
	Tee << "uct size:" << this->uct_nodes_size_ << std::endl;
}
uint32 UCTSearcher::hash_use_rate() const {
	return uint32((double(this->use_node_num_) / double(this->uct_nodes_size_)) * 1000);
}
bool UCTSearcher::is_full() const {
	return hash_use_rate() > 800;
}

void UCTSearcher::think() {
	this->pos_.turn() == BLACK ? this->think<BLACK>()
		: this->think<WHITE>();
}

void UCTSearcher::enqueue_node(const Pos& pos, UCTNode* node) {
	torch::Tensor feat = 
		torch::zeros({ SQUARE_SIZE , POS_END_SIZE, SQUARE_SIZE }, 
					torch::TensorOptions().dtype(torch::kFloat));
	make_feat(pos, feat);
	auto obj = std::make_pair(feat, node);
	this->node_queue_.push(obj);
}

void UCTSearcher::dequeue_node() {
	auto empty = std::queue<std::pair<torch::Tensor, UCTNode * >>();
	std::swap(this->node_queue_, empty);
}

void UCTSearcher::eval_node() {
	
	torch::Tensor all_feat = torch::zeros({ static_cast<int32>(this->node_queue_.size()), SQUARE_SIZE , POS_END_SIZE, SQUARE_SIZE }, torch::TensorOptions().dtype(torch::kFloat));            
	std::vector<UCTNode *> node_list;
	auto i = 0;
	while(!this->node_queue_.empty()) {
		auto pair_info = this->node_queue_.front();
		auto feat = pair_info.first;
		auto node = pair_info.second;
		all_feat[i++] = feat;
		node_list.emplace_back(node);
		this->node_queue_.pop();
	}
	all_feat = all_feat.view({ i, SQUARE_SIZE * POS_END_SIZE ,9,9 });
    torch::Device device(this->device_type_);
	all_feat = all_feat.to(device);
	auto output = this->model_->forward(all_feat);
	auto output_p = std::get<0>(output);
    auto output_v = std::get<1>(output);
	for(auto i = 0u; i < node_list.size(); i++) {
		auto node = node_list[i];
		auto child_node = node->child_;
		std::vector<UCTScore> policy_list;
		for(auto j = 0; j < node->child_num_; j++) {
			auto move = child_node[j].move_;
			auto index = move_to_index(move, node->turn_);
			policy_list.emplace_back(static_cast<UCTScore>(output_p[i][index].item<float>()));
		}

		normalize_by_softmax(policy_list);

		for(auto j = 0; j < node->child_num_; j++) {
			child_node[j].policy_score_ = policy_list[j];
		}
		node->win_score_ = output_v[i][0].item<float>();

		node->evaled_ = true;
	}
}

void normalize_by_softmax(std::vector<UCTScore>& list) {
	auto max_v = std::max_element(list.begin(), list.end());
	UCTScore sum = 0.0f;
	for(auto &x : list) {
		x = static_cast<UCTScore>(expf(static_cast<float>(x) - static_cast<float>(*max_v)));
		sum += x;
	}
	for(auto &x : list) {
		x = x / sum;
	}
}

void start_search(SearchOutput& so, const Pos& pos, const SearchInput& si) {
	Tee << "info think start\n";
	so.init(si, pos);
	gUCT.pos_ = pos;
	gUCT.so_ = &so;
	gUCT.time_limits_.init(si, pos);
	gUCT.think();
	so.end();
	Tee << "info think end\n";

}

static void update_result(ChildNode* child, float result, UCTNode* node) {
	node->win_score_ += result;
	node->po_num_ += 1;
	child->win_score_ += result;
	child->po_num_ += 1;
}

template<Side sd>void UCTSearcher::think() {

	this->init();
	this->expand_root<sd>(this->pos_);
	auto loop = 0ull;
	for (loop = 0ull; ; ++loop) {
		std::vector<std::vector<std::pair<UCTNode *, ChildNode *>>> uct_pv_list; 
		UCTScore score = 0.0f;
		Line pv;
		
		for (auto i = 0; i < 3; i++) {
			pv.clear();
			std::vector<std::pair<UCTNode *, ChildNode *>> uct_pv;
			score = this->uct_search<sd>(this->pos_, &this->root_node_, Ply(0), pv, uct_pv);
			uct_pv_list.emplace_back(uct_pv);
		}

		if (score == score::QUEUEING_SCORE) {
			this->eval_node();
			
			auto result = -1.0f;

			for(auto &uct_pv : uct_pv_list) {
				for(auto i = static_cast<int>(uct_pv.size()-1); i >= 0; i--) {
					auto node_info = uct_pv[i];
					auto curr_node = node_info.first;
					auto child = node_info.second;
					assert(child != nullptr);
					assert(curr_node != nullptr);
					if(i == static_cast<int32>(uct_pv.size()-1)) {
						result = -child->node_ptr_->win_score_;
					}
					update_result(child, result, curr_node);
					result = -result;
				}
			}
		}

		if (this->update_root_info(loop, pv)) {
			this->disp_info(loop, pv, score);
			break;
		}

		if (loop % 100 == 0) {
			this->disp_info(loop, pv, score);
		}
	}
}
bool UCTSearcher::update_root_info(const uint64 loop, const Line& pv) {
	this->so_->move_ = move::MOVE_NONE;
	if (this->root_node_.child_num_ == 0) {
		return true;
	}
	if (this->root_node_.child_num_ == 1) {
		this->so_->move_ = this->root_node_.child_[0].move_;
		this->root_node_.po_num_ = 1;
		this->root_node_.child_[0].po_num_ = 1;
		return true;
	}
	auto max_po = -1;
	auto num = this->root_node_.child_num_;
	ChildNode* child = this->root_node_.child_;

	for (auto i = 0; i < num; ++i) {
		if (child[i].node_ptr_ != nullptr) {
			UCTNode* child_node = child[i].node_ptr_;
			if (child_node->node_state_ == UCTNode::NODE_LOSE) {
				this->so_->move_ = child[i].move_;
				this->so_->pv_ = pv;
				for (auto j = 0; j < num; j++) {
					child[j].po_num_ = 0;
					if (j == i) {
						child[j].po_num_ = this->root_node_.po_num_;
					}
				}
				return true;
			}
		}
		if (child[i].po_num_ > max_po) {
			this->so_->move_ = child[i].move_;
			this->so_->pv_ = pv;
			max_po = child[i].po_num_;
		}
	}
	assert(this->so_->move_ != move::MOVE_NONE);
	//check time up
	double elapsed = this->so_->time();

	if (this->so_->si_->type_ == LIMIT_SMART_TIME || this->so_->si_->type_ == LIMIT_TIME) {
		if (elapsed > this->time_limits_.time_2()) {
			return true;
		}
		if (elapsed > this->time_limits_.time_1()) {
			return true;
		}
		if (elapsed > this->time_limits_.time_0()) {
			return true;
		}
	}
	else if (this->so_->si_->type_ == LIMIT_NODE) {
		if (loop >= this->so_->si_->node_) {
			return true;
		}
	}

	//check input
	if (has_input()) {
		std::string line;
		if (!peek_line(line)) {
			std::exit(EXIT_SUCCESS);
		}
		if (!line.empty()) {
			std::stringstream ss(line);
			std::string command;
			ss >> command;
			if (command == "isready") {
				get_line(line);
				Tee << "readyok" << std::endl;
			}
			else if (command == "stop") {
				get_line(line);
				return true;
			}
		}
	}
	return this->is_full();
}
void UCTSearcher::disp_info(const uint64 loop, const Line& pv, const UCTScore /*sc*/) const {
	double time = this->so_->time();
	std::string line = "info";
	if (time >= 0.001) {
		line += " time " + std::to_string(ml::round(time * 1000));
	}
	if (pv.size() != 0) {
		line += " depth " + std::to_string(pv.size());
	}
	if (loop) {
		line += " nodes " + std::to_string(loop);
	}
	//line += " score cp " + std::to_string(int(sigmoid_inverse(double(sc))));
	if (this->so_->pv_.size() != 0) {
		line += " pv " + this->so_->pv_.to_usi();
	}
	else if (this->so_->move_ != move::MOVE_NONE) {
		line += " pv " + move::move_to_usi(this->so_->move_);
	}
	line += " hashfull " + ml::to_string(this->hash_use_rate());
	Tee << line << std::endl;
	if (pv.size()) {
		line = "";
		std::string line = "info currmove " + move::move_to_usi(pv[0]);
		Tee << line << std::endl;

	}
}
ChildNode* select_child(UCTNode* node, const Pos& /*pos*/) {

	assert(node->child_num_ != 0);

	ChildNode* child = node->child_;
	ChildNode* max_child = nullptr;
	float max_value = -9999999.0f;
	float q, u, ucb_value;

	for (auto i = 0; i < node->child_num_; ++i) {

		if (child[i].node_ptr_ != nullptr) {
			UCTNode* child_node = child[i].node_ptr_;
			if (child_node->node_state_ == UCTNode::NODE_WIN) {
				continue;
			}
			else if (child_node->node_state_ == UCTNode::NODE_LOSE) {
				node->node_state_ = UCTNode::NODE_WIN;
			}
		}
		if (!child[i].po_num_) {
			ucb_value = 99999;
		}
		else {
			q = child[i].win_score_ / child[i].po_num_;
			u = std::sqrt(node->po_num_) / double(1 + child[i].po_num_);
			const auto rate = child[i].policy_score_;
			const auto c_base = 19652;
			const auto c_init = 1.25;
			const auto c = std::log((node->po_num_ + c_base + 1.0f) / c_base) + c_init;
			ucb_value = q + c * u * rate;
		}
		if (ucb_value > max_value) {
			max_value = ucb_value;
			max_child = &child[i];
		}
	}
	return max_child;
}

static uint32 get_index(const Key pos_key, const Key hand_key, const Side sd, const Ply ply) {
	auto ret = uint32(pos_key) ^ uint32(hand_key);
	auto sd_key = hash::key_turn(sd);
	ret ^= uint64(pos_key) >> 32;
	ret ^= uint64(hand_key) >> 32;
	ret ^= uint32(sd_key);
	ret ^= uint64(sd_key) >> 32;
	ret ^= uint32(ply);
	return ret;
}

UCTNode* UCTSearcher::find_same_node(const Key pos_key, const Key hand_key, const Side sd, const Ply ply) {
	uint32 index = get_index(pos_key, hand_key, sd, ply) & this->uct_nondes_mask_;
	auto start_index = index;
	assert(index < this->uct_nodes_size_);
	do {
		if (!this->uct_nodes_[index].used_) {
			return nullptr;
		}
		else if (this->uct_nodes_[index].used_
			&& this->uct_nodes_[index].pos_key_ == pos_key
			&& this->uct_nodes_[index].hand_key_ == hand_key
			&& this->uct_nodes_[index].turn_ == sd
			&& this->uct_nodes_[index].ply_ == ply) {
			return &this->uct_nodes_[index];
		}
		index = this->uct_nondes_mask_ & (index + 1);
	} while (index != start_index);
	return nullptr;
}
UCTNode* UCTSearcher::find_empty_node(const Key pos_key, const Key hand_key, const Side sd, const Ply ply) {
	uint32 index = get_index(pos_key, hand_key, sd, ply) & this->uct_nondes_mask_;
	auto start_index = index;
	do {
		if (!this->uct_nodes_[index].used_) {
			this->uct_nodes_[index].pos_key_ = pos_key;
			this->uct_nodes_[index].hand_key_ = hand_key;
			this->uct_nodes_[index].turn_ = sd;
			this->uct_nodes_[index].ply_ = ply;
			this->uct_nodes_[index].used_ = true;
			this->use_node_num_++;
			return &this->uct_nodes_[index];
		}
		index = this->uct_nondes_mask_ & (index + 1);
	} while (index != start_index);
	return nullptr;
}

template<Side sd>UCTNode* UCTSearcher::expand_node(const Pos& pos, Ply ply) {

	UCTNode* node = this->find_same_node(pos.pos_key(), pos.hand_key(), pos.turn(), ply);

	if (node != nullptr) {
		return node;
	}


	node = this->find_empty_node(pos.pos_key(), pos.hand_key(), pos.turn(), ply);
	node->init(pos, ply);
	ChildNode* child = node->child_;
	List list;
	gen_legals(list, pos);

	auto child_num = 0;
	for (auto i = 0; i < list.size(); i++) {
		child[child_num].move_ = list[i];
		child[child_num].node_ptr_ = nullptr;
		child[child_num].policy_score_ = double(1.0 / double(list.size()));
		child[child_num].win_score_ = 0.0;
		child[child_num].po_num_ = 0;
		child_num++;
	}
	node->child_num_ = child_num;
	return node;
}
template<Side sd> void UCTSearcher::expand_root(const Pos& pos) {

	this->root_node_.init(pos, Ply(0));
	ChildNode* child = this->root_node_.child_;
	List list;
	gen_legals(list, pos);

	auto child_num = 0;
	for (auto i = 0; i < list.size(); i++) {
		child[child_num].move_ = list[i];
		child[child_num].node_ptr_ = nullptr;
		child[child_num].policy_score_ = double(1.0 / double(list.size()));
		child[child_num].win_score_ = 0.0;
		child[child_num].po_num_ = 0;
		child_num++;
	}
	this->root_node_.child_num_ = child_num;
}


template<Side sd> UCTScore UCTSearcher::uct_search(const Pos& pos, UCTNode* node, const Ply ply, Line& pv, std::vector<std::pair<UCTNode *, ChildNode *>>& uct_pv_list) {

#ifdef DEBUG
	if (!pos.is_ok()) {
		Tee << "uct error\n";
		Tee << pos << std::endl;
		exit(EXIT_FAILURE);
	}
	if (pos.turn() != sd) {
		Tee << "not eq sd error\n";
		Tee << pos << std::endl;
		exit(EXIT_FAILURE);
	}
	if (pos.pos_key() != node->pos_key_) {
		Tee << "not eq key error\n";
		Tee << pos << std::endl;
		exit(EXIT_FAILURE);
	}
	if (pos.hand_key() != node->hand_key_) {
		Tee << "not eq  hand_b error\n";
		Tee << pos << std::endl;
		exit(EXIT_FAILURE);
	}
	if (pos.turn() != node->turn_) {
		Tee << "not eq turn error\n";
		Tee << pos << std::endl;
		exit(EXIT_FAILURE);
	}
#endif
	if (!node->child_num_) {
		//Tee<<"mate 1\n";
		//Tee<<pos<<std::endl;
		return -1.0f;
	}
	else if (node->node_state_ == UCTNode::NODE_WIN) {
		//Tee<<"mate 2\n";
		//Tee<<pos<<std::endl;
		return 1.0f;
	}
	else if (node->node_state_ == UCTNode::NODE_LOSE) {
		//Tee<<"mate 3\n";
		//Tee<<pos<<std::endl;
		return -1.0f;
	}
	switch (pos.is_draw()) {
	case Pos::RepCheck:
		//Tee << "check\n";
		return -1.0f;
	case Pos::RepChecked:
		//Tee << "checked\n";
		return 1.0f;
	case Pos::RepEq:
		//Tee << "eq\n";
		return -1.0f;
	case Pos::RepHandWin:
		//Tee << "win\n";
		return 1.0f;
	case Pos::RepHandLose:
		//Tee << "lose\n";
		return -1.0f;
	case Pos::RepNone:
		break;
	}
	if (pos.is_win()) {
		//Tee << "win\n";
		return 1.0f;
	}
	
	Tee<<*node<<std::endl;

	auto* next_child = select_child(node, pos);
	if (next_child == nullptr) {
		node->node_state_ = UCTNode::NODE_LOSE;
		return -1.0f;
	}

	Tee<<*next_child<<std::endl;

	auto new_pos = pos.succ(next_child->move_);

	pv.add(next_child->move_);

	auto result = -1.0f;
	if (next_child->node_ptr_ == nullptr) {
		auto new_node = expand_node<sd>(new_pos, ply);
		auto uct_pv = std::make_pair(node, next_child);
		uct_pv_list.emplace_back(uct_pv);

		assert(new_pos.pos_key() == new_node->pos_key_);
		assert(new_pos.hand_key() == new_node->hand_key_);
		assert(new_pos.turn() == new_node->turn_);
		assert(new_node->turn_ != node->turn_);

		next_child->node_ptr_ = new_node;

		if (new_node->evaled_) {
			result = -(this->uct_search<flip_turn(sd)>(new_pos, new_node, ply + 1, pv, uct_pv_list));
		}
		else if (!new_node->child_num_) {
			new_node->node_state_ = UCTNode::NODE_LOSE;
			result = 1.0f;
		}
		else {
			//new_node->evaled_ = true;
			//return -(uct_eval<flip_turn(sd)>(new_pos));
			enqueue_node(pos,node);
			return score::QUEUEING_SCORE;
		}
	}
	else {
		assert(new_pos.pos_key() == next_child->node_ptr_->pos_key_);
		assert(new_pos.hand_key() == next_child->node_ptr_->hand_key_);
		assert(new_pos.turn() == next_child->node_ptr_->turn_);
#if 0
		if (next_child->node_ptr_->used_ &&
			!next_child->node_ptr_->child_num_) {
			Tee << new_pos << std::endl;
			exit(1);
		}
#endif
		auto uct_pv = std::make_pair(node, next_child);
		uct_pv_list.emplace_back(uct_pv);

		result = -(this->uct_search<flip_turn(sd)>(new_pos, next_child->node_ptr_, ply + 1, pv, uct_pv_list));
	}
	update_result(next_child, result, node);
	return result;
}

std::string UCTSearcher::out_root_info() const {
	const ChildNode* child = this->root_node_.child_;
	const auto num = this->root_node_.child_num_;
	std::string str = "";
	double sum = double(this->root_node_.po_num_);
	for (auto i = 0; i < num; i++) {
		if (!str.empty()) { str += " "; }
		str += move::move_to_usi(child[i].move_) + ":" + ml::to_string(double(child[i].po_num_) / sum);
	}
	return str;
}

namespace uct {
	void test() {
		//Pos pos = pos_from_sfen("9/4k4/9/4P4/9/9/9/9/4K4 b 2G");

		Pos pos = pos_from_sfen(START_SFEN);
		//Pos pos = pos_from_sfen("l6nl/5+P1gk/2np1S3/p1p4Pp/3P2Sp1/1PPb2P1P/P5GS1/R8/LN4bKL w GR5pnsg 1");
		Tee << pos << std::endl;
		gUCT.allocate();
		SearchOutput so;
		SearchInput si;
		si.init();
		si.type_ = LIMIT_DEPTH;
		start_search(so, pos, si);
		//Net model;
		//torch::save(model, "./model.pt");
	}
}
template void UCTSearcher::expand_root<BLACK>(const Pos& pos);
template void UCTSearcher::expand_root<WHITE>(const Pos& pos);
template UCTNode* UCTSearcher::expand_node<BLACK>(const Pos& pos, Ply ply);
template UCTNode* UCTSearcher::expand_node<WHITE>(const Pos& pos, Ply ply);
template UCTScore UCTSearcher::uct_search<BLACK>(const Pos& pos, UCTNode* node, const Ply ply, Line& pv, std::vector<std::pair<UCTNode*, ChildNode*>>& uct_pv_list);
template UCTScore UCTSearcher::uct_search<WHITE>(const Pos& pos, UCTNode* node, const Ply ply, Line& pv, std::vector<std::pair<UCTNode*, ChildNode*>>& uct_pv_list);
