#include "hand.hpp"

std::string hp_to_string(const uint32 hand) {
    std::string s = "";
    PIECE_FOREACH(pc){
      s += ml::rtrim(piece_to_char(pc)) + "/"
        + ml::to_string(num(hand, pc)) + " ";
    }
    return s;
}