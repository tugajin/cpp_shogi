#include "uct.hpp"
#include "pos.hpp"
#include "move.hpp"
#include "common.hpp"
#include "gen.hpp"
#include "list.hpp"
#include "search.hpp"
#include "sfen.hpp"
#include "eval.hpp"
#include <cmath>

void UCTSearcher::init() {
    this->moves_.clear();
    this->uct_nodes_size_ = uint32(1) << 13;
    this->uct_nodes_ = new UCTNode[this->uct_nodes_size_];
    this->uct_nondes_mask_ = this->uct_nodes_size_-1;
    for(auto i = 0; i< this->uct_nodes_size_; ++i) {
        this->uct_nodes_[i].clear();
    }
}

void UCTSearcher::set_pos(const Pos &pos) {
    this->pos_ = pos;
}
void UCTSearcher::think() {
    this->pos_.turn() == BLACK ? this->think<BLACK>() 
                               : this->think<WHITE>();
} 
template<Side sd>void UCTSearcher::think() {
    
    this->init();
    UCTNode * root_node = this->expand_root(this->pos_);
    for(auto loop = 0; loop < 1000; loop++) {
        int pv;
        this->uct_search<sd>(this->pos_, root_node, Ply(0), pv);
    }

}

ChildNode *select_child(UCTNode * node) {
    ChildNode * child = node->child_;
    ChildNode * max_child = nullptr;
    float max_value = -9999999999;
    float q,u;
    for(auto i = 0; i < node->child_num_; ++i) {
        if(!child[i].po_num_) {
            q = 0.9;
            u = 0.9;
        } else {
            q = child[i].win_num_ / child[i].po_num_;
            u = std::sqrt(node->po_num_) / (1 + child[i].po_num_);
        }
        const auto rate = child[i].nn_rate_;
        const auto c_base = 20403;
        const auto c_init = 0.706;
        const auto c = std::log((node->po_num_+c_base + 1.0f)/c_base) + c_init;
        const auto ucb_value = q + c * u * rate;
        if(ucb_value > max_value) {
            max_value = ucb_value;
            max_child = &child[i];
        }
    }
    return max_child;
}

static uint32 get_index(const Key key, const uint32 hand_b, const Side sd, const Ply ply) {
    auto ret = uint32(key);
    auto sd_key = hash::key_turn(sd);
    ret ^= uint64(key) >> 32;
    ret ^= hand_b;
    ret ^=  uint32(sd_key);
    ret ^= uint64(sd_key) >> 32;
    ret ^= uint32(ply);
    return ret;
}

UCTNode * UCTSearcher::find_same_node(const Key key, const uint32 hand_b, const Side sd, const Ply ply){
    uint32 index = get_index(key,hand_b,sd,ply) & this->uct_nondes_mask_;
    auto start_index = index;
    assert(index < this->uct_nodes_size_);
    do {
        if(!this->uct_nodes_[index].used_) {
            return nullptr;
        }
        else if(this->uct_nodes_[index].used_
        && this->uct_nodes_[index].key_ == key 
        && this->uct_nodes_[index].hand_b_ == hand_b
        && this->uct_nodes_[index].sd_ == sd
        && this->uct_nodes_[index].ply_ == ply) {
            return &this->uct_nodes_[index];
        }
        index = this->uct_nondes_mask_ & (index+1);
    } while(index != start_index);
    return nullptr;
}
UCTNode * UCTSearcher::find_empty_node(const Key key, const uint32 hand_b, const Side sd, const Ply ply) {
    uint32 index = get_index(key,hand_b,sd,ply) & this->uct_nondes_mask_;
    auto start_index = index;
    do {
        if(!this->uct_nodes_[index].used_) {
            this->uct_nodes_[index].key_ = key;
            this->uct_nodes_[index].hand_b_ = hand_b;
            this->uct_nodes_[index].sd_ = sd;
            this->uct_nodes_[index].ply_ = ply;
            return &this->uct_nodes_[index];
        }
        index += this->uct_nondes_mask_ & (index+1);
    } while(index != start_index);
    return nullptr;
}

//TODO template
UCTNode * UCTSearcher::expand_node(const Pos &pos, Ply ply) {
    UCTNode *node = this->find_same_node(pos.key(),pos.hand_b(),pos.turn(),ply);
    if(node != nullptr) {
        return node;
    }
    node = this->find_empty_node(pos.key(),pos.hand_b(),pos.turn(),ply);
    node->init(pos,ply);
    ChildNode * child = node->child_;
    List list;
    gen_legals(list,pos);
    auto child_num = 0;
    for(auto i = 0; i < list.size(); i++) {
        child[child_num].move_ = list[i];
        child[child_num].node_ptr_ = nullptr;
        child_num++;
    }
    node->child_num_ = child_num;
    return node;
}
UCTNode * UCTSearcher::expand_root(const Pos &pos) {
    return this->expand_node(pos,Ply(0));
}
static void update_result(ChildNode *child, float result, UCTNode *node) {
    node->win_num_ += result;
    node->po_num_ += 1;
    child->win_num_ += result;
    child->po_num_ += 1;
}

template<Side sd> UCTScore UCTSearcher::uct_search(const Pos &pos, UCTNode *node, const Ply ply, int &pv) {

#ifdef DEBUG
    if(!pos.is_ok()) {
        Tee<<"uct error\n";
        Tee<<pos<<std::endl;
        exit(EXIT_FAILURE);
    }
    if(pos.turn() != sd) {
        Tee<<"not eq sd error\n";
        Tee<<pos<<std::endl;
        exit(EXIT_FAILURE);
    }
#endif
    auto *next_child = select_child(node);
    auto new_pos = pos.succ(next_child->move_);
    auto result = 1.0f;
    if(next_child->node_ptr_ == nullptr) {
        auto new_node = expand_node(new_pos,ply);
        next_child->node_ptr_ = new_node;
        if(new_node->evaled_) {
            result = this->uct_search<flip_turn(sd)>(new_pos, new_node, ply+1,pv);
        } else if(!new_node->child_num_) {
            new_node->score_win_ = -1;
            result = 1.0f;
        } else {
            return eval<sd>(new_pos);
        }
    } else {
        result = this->uct_search<flip_turn(sd)>(new_pos, next_child->node_ptr_, ply+1, pv);
    }
    update_result(next_child,result,node);
    return 1.0f - result;
}

namespace uct {
    void test() {
        Pos pos = pos_from_sfen(START_SFEN);
        UCTSearcher uct;
        uct.init();
        uct.set_pos(pos);
        uct.think();
    }
}

template UCTScore UCTSearcher::uct_search<BLACK>(const Pos &pos, UCTNode *node, const Ply ply, int &pv);
template UCTScore UCTSearcher::uct_search<WHITE>(const Pos &pos, UCTNode *node, const Ply ply, int &pv);
