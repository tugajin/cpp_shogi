#include "uct.hpp"
#include "pos.hpp"
#include "move.hpp"
#include "common.hpp"
#include "gen.hpp"
#include "list.hpp"
#include "search.hpp"

void UCTSearcher::init(const Pos &pos) {
    this->pos_ = pos;
    this->moves_.clear();
    this->uct_nodes_ = new UCTNode[this->uct_nodes_size_];
    this->uct_nondes_mask_ = this->uct_nodes_size_-1;

}
void UCTSearcher::think() {
    this->pos_.turn() == BLACK ? this->think<BLACK>() 
                               : this->think<WHITE>();
} 
template<Side sd>void UCTSearcher::think() {
    
    List list;
    gen_legals(list,this->pos_);

    for(auto i = 0; i < list.size(); i++) {
        this->moves_.push_back(list[i]);
    }

    for(auto loop = 0; loop < 1000; loop++) {
        Line pv;
        this->uct_search<sd>(this->pos_,&this->uct_nodes_[0],Ply(0),pv);
    }
}

ChildNode *select_child(UCTNode * node) {
    for(auto i = 0; i < node->child_num_; i++) {
        return &node->child[i];
    }
    assert(false);
    return nullptr;
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
    uint32 index = get_index(key,hand_b,sd,ply);
    auto start_index = index;
    do {
        if(this->uct_nodes_[index].used_
        && this->uct_nodes_[index].key_ == key 
        && this->uct_nodes_[index].hand_b_ == hand_b
        && this->uct_nodes_[index].sd_ == sd
        && this->uct_nodes_[index].ply_ == ply) {
            return &this->uct_nodes_[index];
        }
        index += this->uct_nondes_mask_ & (index+1);
    } while(index != start_index);
    return nullptr;
}
UCTNode * UCTSearcher::find_empty_node(const Key key, const uint32 hand_b, const Side sd, const Ply ply) {
    uint32 index = get_index(key,hand_b,sd,ply);
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
    ChildNode * child = node->child;
    List list;
    gen_legals(list,pos);
    auto child_num = 0;
    for(auto i = 0; i < list.size(); i++) {
        child[child_num++].move_ = list[i];
    }
    node->child_num_ = child_num;
    return node;
}

static void update_result(ChildNode *child, float result, UCTNode *node) {
    node->win_num_ += result;
    node->po_num_ += 1;
    child->win_num_ += result;
    child->po_num_ += 1;
}

template<Side sd> UCTScore UCTSearcher::uct_search(const Pos &pos, UCTNode *node, const Ply ply, Line &pv) {

#ifdef DEBUG
    if(!pos.is_ok()) {
        Tee<<"uct error\n";
        Tee<<pos<<std::endl;
        exit(EXIT_FAILURE);
    }
#endif
    auto *next_child = select_child(node);
    auto new_pos = pos.succ(next_child->move_);
    auto result = 1.0f;
    if(next_child->node_ptr_ == nullptr) {
        auto new_node = expand_node(pos,ply);
        next_child->node_ptr_ = new_node;
        if(new_node->evaled_) {
            result = this->uct_search<flip_turn(sd)>(pos, new_node, ply+1,pv);
        } else if(!new_node->child_num_) {
            new_node->score_win_ = -1;
            result = 1.0f;
        } else {
            return  1.0f - new_node->score_win_;
        }
    } else {
        result = this->uct_search<flip_turn(sd)>(new_pos, next_child->node_ptr_, ply+1, pv);
    }
    update_result(next_child,result,node);
    return 1.0f - result;
}

template UCTScore UCTSearcher::uct_search<BLACK>(const Pos &pos, UCTNode *node, const Ply ply, Line &pv);
template UCTScore UCTSearcher::uct_search<WHITE>(const Pos &pos, UCTNode *node, const Ply ply, Line &pv);
