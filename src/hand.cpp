#include "hand.hpp"
#include "common.hpp"

std::string hand_to_string(const Hand hand) {
    std::string s = "";
    PIECE_FOREACH(pc){
      if(pc == King) {
        break;
      }
      s += ml::rtrim(piece_to_sfen(pc)) + "/"
        + ml::to_string(hand_num(hand, pc)) + " ";
    }
    return s;
}