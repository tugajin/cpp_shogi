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

UCTSearcher gUCT;
void UCTSearcher::allocate() {
    this->uct_nodes_size_ = (uint32(1) << 19);
    this->uct_nodes_ = new UCTNode[this->uct_nodes_size_];
}
void UCTSearcher::init() {
    this->uct_nondes_mask_ = this->uct_nodes_size_-1;
    this->use_node_num_ = 0;
    for(auto i = 0u; i< this->uct_nodes_size_; ++i) {
        this->uct_nodes_[i].clear();
    }
    Tee<<"uct size:"<<this->uct_nodes_size_<<std::endl;
}

bool UCTSearcher::is_full() const {
    auto use_rate = double(this->use_node_num_) / double(this->uct_nodes_size_);
    return use_rate > 0.8;
}

void UCTSearcher::set_pos(const Pos &pos) {
    this->pos_ = pos;
}
void UCTSearcher::think() {
    this->pos_.turn() == BLACK ? this->think<BLACK>() 
                               : this->think<WHITE>();
} 

void start_search(SearchOutput & so, const Pos &pos, const SearchInput &si) {
    so.init(si,pos);
    gUCT.set_pos(pos);
    gUCT.think();
    so.end();
}
template<Side sd>void UCTSearcher::think() {
    
    this->init();
    this->expand_root<sd>(this->pos_);
    for(auto loop = 0; loop < 1000000; loop++) {
        Line pv;
        pv.clear();
        this->uct_search<sd>(this->pos_, &this->root_node_, Ply(0), pv);
        Tee<<pv.to_usi()<<std::endl;
        if(loop % 500 == 0) {
            if(this->is_full()) {
                Tee<<"full uct\n";
                break;
            }
        }
    }

}

ChildNode *select_child(UCTNode * node) {
    
    assert(node->child_num_ != 0);

    ChildNode * child = node->child_;
    ChildNode * max_child = nullptr;
    float max_value = -9999999999;
    float q,u, ucb_value;
    for(auto i = 0; i < node->child_num_; ++i) {
        if(!child[i].po_num_) {
            ucb_value = 99999;
        } else {
            q = child[i].win_score_ / child[i].po_num_;
            u = std::sqrt(node->po_num_) / (1 + child[i].po_num_);
            const auto rate = child[i].policy_score_;
            const auto c_base = 20403;
            const auto c_init = 0.706;
            const auto c = std::log((node->po_num_+c_base + 1.0f)/c_base) + c_init;
            ucb_value = q + c * u * rate;
        
        }
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
        && this->uct_nodes_[index].turn_ == sd
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
            this->uct_nodes_[index].turn_ = sd;
            this->uct_nodes_[index].ply_ = ply;
            this->uct_nodes_[index].used_ = true;
            this->use_node_num_++;
            return &this->uct_nodes_[index];
        }
        index = this->uct_nondes_mask_ & (index+1);
    } while(index != start_index);
    return nullptr;
}

template<Side sd>UCTNode * UCTSearcher::expand_node(const Pos &pos, Ply ply) {

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
template<Side sd> void UCTSearcher::expand_root(const Pos &pos) {

    this->root_node_.init(pos,Ply(0));
    ChildNode * child = this->root_node_.child_;
    List list;
    gen_legals(list,pos);
    
    auto child_num = 0;
    for(auto i = 0; i < list.size(); i++) {
        child[child_num].move_ = list[i];
        child[child_num].node_ptr_ = nullptr;
        child_num++;
    }
    this->root_node_.child_num_ = child_num;
}
static void update_result(ChildNode *child, float result, UCTNode *node) {
    node->win_score_ += result;
    node->po_num_ += 1;
    child->win_score_ += result;
    child->po_num_ += 1;
}

template<Side sd> UCTScore UCTSearcher::uct_search(const Pos &pos, UCTNode *node, const Ply ply, Line &pv) {

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
    if(pos.key() != node->key_) {
        Tee<<"not eq key error\n";
        Tee<<pos<<std::endl;
        exit(EXIT_FAILURE);
    }
    if(pos.hand_b() != node->hand_b_) {
        Tee<<"not eq  hand_b error\n";
        Tee<<pos<<std::endl;
        exit(EXIT_FAILURE);
    }
    if(pos.turn() != node->turn_) {
        Tee<<"not eq turn error\n";
        Tee<<pos<<std::endl;
        exit(EXIT_FAILURE);
    }
#endif
    if(!node->child_num_) {
        return 1.0f;
    } else if(node->node_state_ == UCTNode::NODE_WIN) {
        return 0.0f;
    } else if(node->node_state_ == UCTNode::NODE_LOSE) {
        return 1.0f;
    }
    auto *next_child = select_child(node);
    if(ply == Ply(0)) {
        Tee<<"select:"<<move::move_to_string(next_child->move_)<<" full:"<<double(this->use_node_num_) / double(this->uct_nodes_size_)<<std::endl;
    }
#ifdef DEBUG
    if(next_child == nullptr) {
        Tee<<pos<<std::endl;
        exit(EXIT_FAILURE);
    }
#endif

    auto new_pos = pos.succ(next_child->move_);
    
    pv.add(next_child->move_);
    
    auto result = 1.0f;
    if(next_child->node_ptr_ == nullptr) {
        auto new_node = expand_node<sd>(new_pos,ply);
        assert(new_pos.key() == new_node->key_);
        assert(new_pos.hand_b() == new_node->hand_b_);
        assert(new_pos.turn() == new_node->turn_);
        assert(new_node->turn_ != node->turn_);

        next_child->node_ptr_ = new_node;

        if(new_node->evaled_) {
            result = this->uct_search<flip_turn(sd)>(new_pos, new_node, ply+1,pv);
        } else if(!new_node->child_num_) {
            new_node->node_state_ = UCTNode::NODE_LOSE;
            result = 1.0f;
        } else {
            new_node->evaled_ = true;
            return uct_eval<sd>(new_pos);
        }
    } else {
        assert(new_pos.key() == next_child->node_ptr_->key_);
        assert(new_pos.hand_b() == next_child->node_ptr_->hand_b_);
        assert(new_pos.turn() == next_child->node_ptr_->turn_);
#if 0
        if(next_child->node_ptr_->used_ && 
           !next_child->node_ptr_->child_num_) {
               Tee<<new_pos<<std::endl;
               exit(1);
        }
#endif
        result = this->uct_search<flip_turn(sd)>(new_pos, next_child->node_ptr_, ply+1, pv);
    }
    update_result(next_child,result,node);
    return 1.0f - result;
}

namespace uct {
    void test() {
        //Pos pos = pos_from_sfen("9/4k4/9/4P4/9/9/9/9/4K4 b 2G");
        
        //Pos pos = pos_from_sfen(START_SFEN);
        Pos pos = pos_from_sfen("l6nl/5+P1gk/2np1S3/p1p4Pp/3P2Sp1/1PPb2P1P/P5GS1/R8/LN4bKL w GR5pnsg 1");
        Tee<<pos<<std::endl;
        gUCT.allocate();
        gUCT.set_pos(pos);
        gUCT.think();
    }
}
template void UCTSearcher::expand_root<BLACK>(const Pos &pos);
template void UCTSearcher::expand_root<WHITE>(const Pos &pos);
template UCTNode * UCTSearcher::expand_node<BLACK>(const Pos &pos, Ply ply);
template UCTNode * UCTSearcher::expand_node<WHITE>(const Pos &pos, Ply ply);
template UCTScore UCTSearcher::uct_search<BLACK>(const Pos &pos, UCTNode *node, const Ply ply, Line &pv);
template UCTScore UCTSearcher::uct_search<WHITE>(const Pos &pos, UCTNode *node, const Ply ply, Line &pv);
