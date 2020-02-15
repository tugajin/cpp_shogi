#include "search.hpp"
#include "attack.hpp"
#include "gen.hpp"
#include "move.hpp"
#include "sfen.hpp"
#include "list.hpp"

void SearchInput::init() {
    
}

template<Side sd> static uint64 perft(const Pos &pos, Ply ply) {
    assert(pos.is_ok());
#ifdef DEBUG
    if(!is_legal(pos)) {
        Tee<<pos<<std::endl;
        Tee<<move::move_to_string(pos.last_move())<<std::endl;
    }
#endif
    assert(is_legal(pos));

    if(ply <= 0) {
        return 1;
    }
    List list;
    gen_moves<sd>(list,pos);
    uint64 num = 0;
    for(auto i = 0; i < list.size(); ++i) {
        auto mv = list[i];
        assert(move::move_is_ok(mv,pos));
        if(!move::pseudo_is_legal(mv,pos)) {
            continue;
        }
        auto new_pos = pos.succ(mv);
        num += perft<flip_turn(sd)>(new_pos,ply-1);
    }
    return num;
}

template uint64 perft<BLACK>(const Pos &pos, Ply ply);
template uint64 perft<WHITE>(const Pos &pos, Ply ply);

namespace search {
    void test() {
        Pos pos = pos_from_sfen(START_SFEN);
        auto num = perft<BLACK>(pos,Ply(5));
        std::cout<<num<<std::endl;
    }
}