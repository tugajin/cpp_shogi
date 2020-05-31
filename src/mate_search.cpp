#include "mate_search.hpp"
#include "common.hpp"
#include "move.hpp"
#include "attack.hpp"
#include "pos.hpp"
#include "list.hpp"
#include "gen.hpp"
#include "sfen.hpp"

enum MatePoint {
    OR_POINT, //玉方
    AND_POINT, //攻方
};

inline constexpr MatePoint flip_matepoint(const MatePoint p) {
    return (p == OR_POINT) ? AND_POINT : OR_POINT;
}

template<Side sd, MatePoint mp> bool search(const Pos &pos, const Ply ply) {
 
    assert(pos.is_ok());

    if(ply < Ply(0)) {
        //Tee<<int(ply)<<"limit\n";
        return false;
    }

    List list;
    if(mp == OR_POINT) {
        //Tee<<int(ply)<<"evasion\n";
        const auto chk = checks(pos);
        gen_moves<EVASION,sd>(list,pos,&chk);
    } else {
        //Tee<<int(ply)<<"check\n";
        gen_moves<CHECK,sd>(list,pos,nullptr);
    }
    //Tee<<pos<<std::endl;
    for(auto i = 0; i < list.size(); i++) {
        const auto mv = list.move(i);
        const auto new_pos = pos.succ(mv);
        if(!is_legal(new_pos)) {
            continue;
        }
        //Tee<<int(ply)<<"try:"<<move::move_to_string(mv)<<std::endl;
        const auto result = search<flip_turn(sd),flip_matepoint(mp)>(new_pos,ply-1);
        if(mp == OR_POINT) {
            if(!result) {
                //Tee<<int(ply)<<"return OR false\n";
                return false;
            }
        } else {
            if(result) {
                //Tee<<int(ply)<<"return AND true\n";
                return true;
            }
        }
    }
    if (mp == OR_POINT) {
        //Tee<<int(ply)<<"return OR true\n";
        return true;
    } else {
        //Tee<<int(ply)<<"return AND false\n";
        return false;
    }
}

template<Side sd> Move mate_search(const Pos &pos, const Ply ply) {
    
    assert(!in_check(pos));
    assert(pos.is_ok());

    List list;
    gen_moves<CHECK,sd>(list,pos,nullptr);
    //Tee<<"list:size:"<<list.size()<<std::endl;
    for(auto limit_ply = 1; limit_ply < ply; limit_ply++) {
        for(auto i = 0; i < list.size(); i++) {
            const auto mv = list.move(i);
            const auto new_pos = pos.succ(mv);
            if(!is_legal(new_pos)) {
                continue;
            }
            //Tee<<"root:"<<move::move_to_string(mv)<<std::endl;
            const auto result = search<flip_turn(sd),OR_POINT>(new_pos,ply-1);
            //Tee<<"root result:"<<result<<std::endl;
            
            if(result) {
                return mv;
            }
        }
    }
    return move::MOVE_NONE;
}

Move mate_search(const Pos &pos, const Ply ply) {
    return pos.turn() == BLACK ? mate_search<BLACK>(pos,ply) : mate_search<WHITE>(pos,ply);
}


namespace mate {

    void test() {
        {
			Pos pos = pos_from_sfen("4k4/9/4P4/9/9/9/9/9/4K4 b G");
			Tee << pos << std::endl;
            auto mv = move::MOVE_NONE;
            mv = mate_search<BLACK>(pos,Ply(3));
            Tee<<move::move_to_string(mv)<<std::endl;
		}
        {
			Pos pos = pos_from_sfen("9/4k4/9/4P4/9/9/9/9/4K4 b 2G");
			Tee << pos << std::endl;
            auto mv = move::MOVE_NONE;
            mv = mate_search<BLACK>(pos,Ply(3));
            Tee<<move::move_to_string(mv)<<std::endl;
		}

    }

}