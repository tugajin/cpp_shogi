#include <string>
#include "move.hpp"
#include "pos.hpp"

namespace move {

Move from_usi(const std::string &s, const Pos &pos) {
    Move(0);
}

std::string move_to_usi(const Move mv) {
    if (mv == MOVE_NONE || mv == MOVE_NULL) {
      return "0000";
    }
    std::string s;
    if (move_is_drop(mv)) {
      s += ml::rtrim(piece_to_char(move_piece(mv)));
      s += "*";
      s += sq_to_string(move_to(mv));
    } else {
      s += sq_to_string(move_from(mv));
      s += sq_to_string(move_to(mv));
      if (move_is_prom(mv)) {
        s += "+";
      }
    }
    return s;
}


}