#ifndef UCT_HPP
#define UCT_HPP

#include "pos.hpp"
#include "move.hpp"
#include "search.hpp"
#include "thread.hpp"
#include "nn.hpp"
#include <vector>
#include <queue>
#include <utility>

class UCTNode;

class Time {

private:

	double time_0_; // target
	double time_1_; // extended
	double time_2_; // maximum

public:

	static double time_lag(double time) {
		return std::max(time - 0.1, 0.0); // assume 100ms of lag
	}

	void init(const SearchInput& si, const Pos& pos) {
		if (si.type_ == LIMIT_SMART_TIME) {
			this->init(si.moves_, si.time_, si.inc_, si.byoyomi_, pos);
		}
		else {
			this->init(si.time_ + si.byoyomi_);
		}
	}

	double time_0() const { return time_0_; }
	double time_1() const { return time_1_; }
	double time_2() const { return time_2_; }

private:

	void init(double time) {
		this->time_0_ = time;
		this->time_1_ = time;
		this->time_2_ = time;
	}
	void init(int moves, double time, double inc, double byoyomi, const Pos& /*pos*/) {

		moves = std::min(moves, 30);

		double moves_left = double(120 - moves);
		double factor = 1.3;
		//if (var::Ponder) factor *= 1.2;

		double total = std::max(time + inc * moves_left, 0.0);
		double alloc = total / moves_left * factor;

		if (moves > 1) { // save some time for the following moves
			double total_safe = std::max((time / double(moves - 1) + inc - (time / double(moves) + inc) * 0.5) * double(moves - 1), 0.0);
			total = std::min(total, total_safe);
		}

		double max = time_lag(std::min(total, time + inc) * 0.95);

		if (time == 0.0) {
			alloc += byoyomi;
			max += byoyomi;
		}

		this->time_0_ = std::min(time_lag(alloc), max);
		this->time_1_ = std::min(time_lag(alloc * 4.0), max);
		this->time_2_ = max;

		assert(0.0 <= time_0_ && time_0_ <= time_1_ && time_1_ <= time_2_);
	}
};

class ChildNode {
public:
	Move move_;
	int po_num_;
	float win_score_;
	float policy_score_;
	UCTNode* node_ptr_;
	friend std::ostream& operator<<(std::ostream& os, const ChildNode& node) {
		os<<"move:"<<move::move_to_string(node.move_)<<std::endl;
		os<<"po_num:"<<node.po_num_<<std::endl;
		os<<"win_score:"<<node.win_score_<<std::endl;
		os<<"policy_score:"<<node.win_score_<<std::endl;
		return os;
	}
};


class UCTNode {
public:
	enum UCTNodeState { NODE_LOSE = -1, NODE_WIN = 1, NODE_UNKNOWN = 0 };

	Key pos_key_;
	Key hand_key_;
	Side turn_;
	Ply ply_;
	int po_num_;
	float win_score_;
	UCTNodeState node_state_;
	int child_num_;
	bool evaled_;
	bool is_draw_;
	bool used_;
	ChildNode child_[MAX_LEGAL_MOVES];
	Lockable node_lock_;

	void clear() {
		this->po_num_ = 0;
		this->win_score_ = 0.0f;
		this->evaled_ = false;
		this->is_draw_ = false;
		this->node_state_ = UCTNode::NODE_UNKNOWN;
		this->used_ = false;
	}
	void init(const Pos& pos, const Ply ply) {
		this->clear();
		this->pos_key_ = pos.pos_key();
		this->hand_key_ = pos.hand_key();
		this->turn_ = pos.turn();
		this->ply_ = ply;
		this->used_ = true;
	}
	friend std::ostream& operator<<(std::ostream& os, const UCTNode& node) {
		os<<"key:"<<static_cast<uint64>(node.pos_key_)<<std::endl;
		os<<"hand:"<<static_cast<uint64>(node.hand_key_)<<std::endl;
		os<<"turn:"<<static_cast<int>(node.turn_)<<std::endl;
		os<<"ply:"<<static_cast<int>(node.ply_)<<std::endl;
		os<<"used:"<<node.used_<<std::endl;
		os<<"po_num:"<<node.po_num_<<std::endl;
		os<<"win_score:"<<node.win_score_<<std::endl;
		os<<"node_state:"<<static_cast<int>(node.node_state_)<<std::endl;
		os<<"child_num:"<<node.child_num_<<std::endl;
		os<<"evaled:"<<node.evaled_<<std::endl;
		os<<"draw:"<<node.is_draw_<<std::endl;
		os<<"children:\n";
		for(auto i = 0; i < node.child_num_; i++) {
			os<<node.child_[i]<<std::endl;
		}
		return os;
	}
};

struct EvalNodeList {
	static constexpr uint32 LIST_SIZE = 512;
	float feat[LIST_SIZE][POS_END_SIZE][FILE_SIZE][RANK_SIZE];
	UCTNode* node_list[LIST_SIZE];
	uint32 curr;
};

class UCTSearcher {
public:
	Pos pos_;
	SearchOutput* so_;
	Time time_limits_;
	Lockable tree_lock_;
	
	void think();
	void init();
	void allocate();
	void free();
	std::string out_root_info() const;

private:
	template<Side sd> void think();
	template<Side sd> UCTScore uct_search(const Pos& pos, UCTNode* node, const Ply ply, Line& pv, std::vector<std::pair<UCTNode*, ChildNode*>>& uct_pv_list);
	UCTNode* find_same_node(const Key key, const Key hand_key, const Side sd, const Ply ply);
	UCTNode* find_empty_node(const Key key, const Key hand_key, const Side sd, const Ply ply);
	template<Side sd> void expand_root(const Pos& pos);
	template<Side sd> UCTNode* expand_node(const Pos& pos, Ply ply);
	uint32 hash_use_rate() const;
	bool is_full() const;
	bool update_root_info(const uint64 loop, const Line& pv);
	void enqueue_node(const Pos& pos, UCTNode* node);
	void reset_queue();
	void eval_node();
	void disp_info(const uint64 loop, const Line& pv, const UCTScore sc)const;
	uint32 uct_nodes_size_;
	uint32 uct_nondes_mask_;
	uint32 use_node_num_;
	UCTNode root_node_;
	UCTNode* uct_nodes_ = nullptr;
	//Lockable eval_node_lock_;
	EvalNodeList eval_node_list_;
};

void start_search(SearchOutput& so, const Pos& pos, const SearchInput& si);
void normalize_by_softmax(std::vector<UCTScore>& list);

extern UCTSearcher gUCT;

namespace uct {
	void test();
}

#endif
