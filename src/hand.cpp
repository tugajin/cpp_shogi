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
bool hand_is_ok(const Hand h) {
  auto hand = uint32(h);
  const auto over_flow = hand & HAND_OVERFLOW_MASK;
  if(over_flow) {
    return false;
  }
  auto pawn_num = hand_num(h,Pawn);
  if(pawn_num > 18) { return false;}
  auto lance_num = hand_num(h,Lance);
  if(lance_num > 4) { return false;}
  auto knight_num = hand_num(h,Knight);
  if(knight_num > 4) { return false;}
  auto silver_num = hand_num(h,Silver);
  if(silver_num > 4) { return false;}
  auto gold_num = hand_num(h,Gold);
  if(gold_num > 4) { return false;}
  auto bishop_num = hand_num(h,Bishop);
  if(bishop_num > 2) { return false;}
  auto rook_num = hand_num(h,Rook);
  if(rook_num > 2) { return false;}

  return true;
}
