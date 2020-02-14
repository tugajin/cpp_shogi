#include <string>
#include "move.hpp"
#include "pos.hpp"
#include "attack.hpp"

namespace move {

bool pseudo_is_legal(const Move mv, const Pos &pos) {
  assert(mv != move::MOVE_NONE);
  assert(mv != move::MOVE_NULL);
  if(move::move_is_drop(mv)) {
    return true;
  }
  const auto from = move::move_from(mv);
  const auto to = move::move_to(mv);
  const auto pc = move::move_piece(mv);
  const auto sd = pos.turn();
  const auto xd  = flip_turn(sd);
  const auto king = pos.king(sd);
  auto pieces = pos.pieces();
  
  pieces.clear(from);
  pieces.set(to);

  //king move
  if (pc == King) {
    return has_attack(pos,xd,to,pieces);
  }
  //pinned piece
  auto beyond = bit::beyond(king,from);

  auto b = (pos.pieces(Bishop) | pos.pieces(PBishop)) & pos.pieces(xd) & beyond;
  while(b) {
    const auto ds = b.lsb();
    if(ds != to && piece_attack<Bishop>(xd,ds,king,pieces)) {
      return false;
    }
  }
  b = (pos.pieces(Rook) | pos.pieces(PRook)) & pos.pieces(xd) & beyond;
  while(b) {
    const auto ds = b.lsb();
    if(ds != to && piece_attack<Rook>(xd,ds,king,pieces)) {
      return false;
    }
  }
  b = pos.pieces(Lance,xd) & g_lance_mask[sd][king];
  if(b) {
    const auto ds = b.lsb<false>();
    if(ds != to && line_is_empty(ds,king,pieces)) {
      return false;
    }
  }

  return true;

}

bool is_check(const Move mv, const Pos &pos) {
  return false;
}

Move from_usi(const std::string &s, const Pos &pos) {
    if (std::isdigit(s[0])) { //move
      const auto from = sq_from_string(s.substr(0, 2));
      const auto to = sq_from_string(s.substr(2, 2));
      const auto pc = pos.piece(from);
      const auto cp = pos.piece(to);
      const auto prom = s.find("+") != std::string::npos;
      return move::make_move(from, to, pc, cp, prom);
    } else { //drop
      const auto pc = sfen_to_piece(s.substr(0,1));
      const auto to = sq_from_string(s.substr(2,2));
      return move::make_move(to, pc);
    }
}

std::string move_to_usi(const Move mv) {
    if (mv == MOVE_NONE || mv == MOVE_NULL) {
      return "0000";
    }
    std::string s = "";
    if (move_is_drop(mv)) {
      s += ml::rtrim(piece_to_sfen(move_piece(mv)));
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

std::string move_to_string(const Move mv) {
    if (mv == MOVE_NONE) {
      return "0000";
    }
    if (mv == MOVE_NULL) {
      return "NULL";
    }
    std::string s = "";
    if (move_is_drop(mv)) {
      s += "drop:";
      s += ml::rtrim(piece_to_sfen(move_piece(mv)));
      s += " piece:";
      s += sq_to_string(move_to(mv));
    } else {
      s += "from:";
      s += sq_to_string(move_from(mv));
      s += " to:";
      s += sq_to_string(move_to(mv));
      s += " piece:";
      s += ml::rtrim(piece_to_sfen(move_piece(mv)));
      s += " cap:";
      s += ml::rtrim(piece_to_sfen(move_cap(mv)));
      if (move_is_prom(mv)) {
        s += "+";
      }
    }
    return s;
}

}