#ifndef UCT_HPP
#define UCT_HPP

#include "pos.hpp"
#include "move.hpp"
#include "search.hpp"
#include <vector>

class UCTNode;

class ChildNode {
public:
    Move move_;
    int po_num_;
    float win_num_;
    UCTNode *node_ptr_;
    UCTScore nn_rate_;
};

class UCTNode {
public:
    Key key_;
    uint32 hand_b_;
    Side sd_;
    Ply ply_;
    int po_num_;
    float win_num_;
    UCTScore score_win_;
    float nn_rate_;
    int child_num_;
    ChildNode child_[MAX_LEGAL_MOVES];
    
    bool evaled_;
    bool is_draw_;
    bool used_;
    void clear() {
        this->po_num_ = 0;
        this->win_num_ = 0.0f;
        this->evaled_ = false;
        this->is_draw_ = false;
        this->score_win_ = 0.0f;
        this->nn_rate_ = 0.0f;
        this->used_ = false;
    }
    void init(const Pos &pos,const Ply ply) {
        this->clear();
        this->key_ = pos.key();
        this->hand_b_ = pos.hand_b();
        this->sd_ = pos.turn();
        this->ply_ = ply;
        this->used_ = true;
    }
};

class UCTSearcher {
public:
    Pos pos_;
    void think();
    void init();
    void set_pos(const Pos &pos);
private:
    template<Side sd> void think();
    template<Side sd> UCTScore uct_search(const Pos &pos, UCTNode *node, const Ply ply, Line &pv);
    UCTNode * find_same_node(const Key key, const uint32 hand_b, const Side sd, const Ply ply);
    UCTNode * find_empty_node(const Key key, const uint32 hand_b, const Side sd, const Ply ply);
    UCTNode * expand_root(const Pos &pos);
    UCTNode * expand_node(const Pos &pos, Ply ply);
    uint32 uct_nodes_size_;
    uint32 uct_nondes_mask_;
    UCTNode * uct_nodes_;
    uint32 use_node_num_;
};

namespace uct {
    void test();
}

#endif