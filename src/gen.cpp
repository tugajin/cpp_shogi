#include "gen.hpp"
#include "bit.hpp"
#include "pos.hpp"
#include "list.hpp"

enum MoveType {
    TACTICAL, QUIET, DROP, EVASION,
};
template<Piece pc, MoveType mt, Side sd>void add_noprom_move(List & ml, const Pos & pos,const bit::Bitboard & target) {
    auto piece =(pc == Golds) ? pos.golds(sd) : pos.pieces(pc, sd);
    while(piece) {
        const auto from = piece.lsb();
        for (auto att = piece_attacks<pc>(sd, from, pos.pieces())& target; bool(att);) {
          const auto to = att.lsb();
          ml.add(move::make_move(from, to, pos.piece(from), pos.piece(to)));
        }
    }
}
template<MoveType mt, Side sd>void add_king_move(List & ml, const Pos & pos,const bit::Bitboard & target) {
      const auto from = pos.king(sd);
        for (auto att = get_king_attack(from) & target;bool(att);) {
        const auto to = att.lsb();
        ml.add(move::make_move(from, to, King, pos.piece(to)));
    }
}
template<MoveType mt, Side sd>void add_pawn_move(List & ml, const Pos & pos,const bit::Bitboard & target) {
    for (auto att = get_pawn_attack<sd>( pos.pieces(Pawn, sd)) & target; bool(att);) {
        const auto to = att.lsb();
        const auto from =(sd == BLACK) ? to + SQ_RANK_INC : to - SQ_RANK_INC;
        ml.add( move::make_move(from, to, Pawn, pos.piece(to),square_is_prom<sd>(to)));
    }
}
template<Piece pc, MoveType mt, Side sd>void add_bishop_rook_move(List & ml, const Pos & pos,const bit::Bitboard & target) {
    static_assert(pc == Bishop || pc == Rook,"pc error");
    for(auto piece = pos.pieces(pc,sd); bool(piece);) {
        const auto from = piece.lsb();
        for (auto att = piece_attacks<pc>(sd, from, pos.pieces()) & target; bool(piece);) {
          const auto to = att.lsb();
          const auto prom = square_is_prom<sd>(from) || square_is_prom<sd>(to);
          ml.add(move::make_move(from, to, pos.piece(from), pos.piece(to), prom));
        }
    }
}
template<MoveType mt, Side sd>void add_silver_move(List & ml, const Pos & pos,const bit::Bitboard & target) {
    for (bit::Bitboard piece = pos.pieces(Silver, sd); bool(piece);) {
        const auto from = piece.lsb();
        for (auto att = piece_attacks<Silver>(sd, from,pos.pieces()) & target; bool(att);) {
          const auto to = att.lsb();
          const auto prom = square_is_prom<sd>(from) || square_is_prom<sd>(to);
          if (prom) {
            ml.add(move::make_move(from, to, Silver, pos.piece(to),true));
          }
          ml.add(move::make_move(from, to, Silver, pos.piece(to), false));
        }
    }
}
template<Piece pc, MoveType mt, Side sd>void add_lance_knight_move(List & ml, const Pos & pos, const bit::Bitboard & target) {
    static_assert(pc == Lance || pc == Knight, "pc error");
    for (auto piece = pos.pieces(pc, sd); bool(piece);) {
        const auto from = piece.lsb();
        for (auto att = piece_attacks<pc>(sd, from, pos.pieces()) & target; bool(att);) {
            const auto to = att.lsb();
            const auto to_rank = square_rank<sd>(to);
            if (to_rank == Rank_1 || to_rank == Rank_2) {
                ml.add(move::make_move(from, to, pos.piece(from), pos.piece(to),true));
            } else if (to_rank == Rank_3) {
                ml.add(move::make_move(from, to, pos.piece(from), pos.piece(to),true));
                ml.add(move::make_move(from, to, pos.piece(from), pos.piece(to),false));
            } else {
                ml.add(move::make_move(from, to, pos.piece(from), pos.piece(to),false));
            }
        }
    }
}
template<bool has_knight, bool has_lance, bool has_pawn, Side sd, int num>
    void add_drop_move(List & ml, const Pos & pos,const bit::Bitboard & target, const int p[]) {
      if (has_pawn) {
        //pawn
        const auto mask = bit::g_rank_mask[square::RANK_1]
                        | bit::g_rank_mask[square::RANK_2]
                        | bit::g_rank_mask[square::RANK_3]
                        | bit::g_rank_mask[square::RANK_4]
                        | bit::g_rank_mask[square::RANK_5]
                        | bit::g_rank_mask[square::RANK_6]
                        | bit::g_rank_mask[square::RANK_7]
                        | bit::g_rank_mask[square::RANK_8];
        auto pawn = bd.pieces(Pawn, sd) + mask;
        const auto index = (uint32) (ml::pext(pawn.p<0>(),bit::g_rank_mask[Rank_9].p<0>())
                                  + (ml::pext(pawn.p<1>(),bit::g_rank_mask[Rank_9].p(1)) << 7));
        //std::cout<<"index:"<<index<<std::endl;
        //std::cout<<bit::g_double_pawn_mask[sd][index]<<std::endl;
        auto pawn_target = target & bit::g_double_pawn_mask[sd][index];
        const auto mate_with_sq = (sd == BLACK) ?
          bd.king(flip_turn(sd)) - SQ_RANK_INC :
          bd.king(flip_turn(sd)) + SQ_RANK_INC;
        if (pawn_target.is_set(mate_with_sq)
            && is_mate_with_pawn_drop(mate_with_sq, bd)) {
          pawn_target.clear(mate_with_sq);
        }
        while (bool(pawn_target)) {
          ml.add(move::make_move(pawn_target.lsb(), Pawn));
        }

      }

      //rank1
      bit::Bitboard rank_1 = target
        & bit::g_rank_mask[square::side_rank<sd>(square::RANK_1)];
      while (!rank_1.is_empty()) {
        const auto to = rank_1.lsb();
        for (auto i = 0; i < num; i++) {
          ml.add(move::make(to, p[i]));
        }
      }

      //rank2
      bit::Bitboard rank_2 = target
        & bit::g_rank_mask[square::side_rank<sd>(square::RANK_2)];
      while (!rank_2.is_empty()) {
        const auto to = rank_2.lsb();
        for (auto i = 0; i < num; i++) {
          ml.add(move::make(to, p[i]));
        }
        if (has_lance) {
          ml.add(move::make(to, piece::LANCE));
        }
      }

      //rank3~rank9
      bit::Bitboard rank_39 = target
        & ~(bit::g_rank_mask[square::side_rank<sd>(square::RANK_1)]
            | bit::g_rank_mask[square::side_rank<sd>(square::RANK_2)]);
      while (!rank_39.is_empty()) {
        const auto to = rank_39.lsb();
        for (auto i = 0; i < num; i++) {
          ml.add(move::make(to, p[i]));
        }
        if (has_lance) {
          ml.add(move::make(to, piece::LANCE));
        }
        if (has_knight) {
          ml.add(move::make(to, piece::KNIGHT));
        }
      }
    }
  template<int sd>
    void add_drop_move(List & ml, const board::Board & bd,
        const bit::Bitboard & target) {

      assert(bd.is_ok());

      const auto hand = bd.hand(sd);
      auto status = 0;
      int piece_list[4] = { };
      int * p = piece_list;

      (hand::has(hand, piece::PAWN)) ? status += 1 : status;
      (hand::has(hand, piece::LANCE)) ? status += 2 : status;
      (hand::has(hand, piece::KNIGHT)) ? status += 4 : status;
      (hand::has(hand, piece::SILVER)) ?
        status += 8, (*p++) = piece::SILVER : status;
      (hand::has(hand, piece::GOLD)) ? status += 8, (*p++) = piece::GOLD : status;
      (hand::has(hand, piece::BISHOP)) ?
        status += 8, (*p++) = piece::BISHOP : status;
      (hand::has(hand, piece::ROOK)) ? status += 8, (*p++) = piece::ROOK : status;

#define ADD_DROP_MOVE(n) {\
  constexpr auto knight_flag = (n & (1 << 2)) != 0;\
  constexpr auto lance_flag  = (n & (1 << 1)) != 0;\
  constexpr auto pawn_flag   = (n & (1 << 0)) != 0;\
  constexpr auto num = n >> 3;\
  add_drop_move<knight_flag,lance_flag,pawn_flag,sd,num>(ml,bd,target,piece_list); \
  break;\
}
      switch (status) {
        case 0:
          break;
        case 1:
          ADD_DROP_MOVE(1)
        case 2:
            ADD_DROP_MOVE(2)
        case 3:
              ADD_DROP_MOVE(3)
        case 4:
                ADD_DROP_MOVE(4)
        case 5:
                  ADD_DROP_MOVE(5)
        case 6:
                    ADD_DROP_MOVE(6)
        case 7:
                      ADD_DROP_MOVE(7)
        case 8:
                        ADD_DROP_MOVE(8)
        case 9:
                          ADD_DROP_MOVE(9)
        case 10:
                            ADD_DROP_MOVE(10)
        case 11:
                              ADD_DROP_MOVE(11)
        case 12:
                                ADD_DROP_MOVE(12)
        case 13:
                                  ADD_DROP_MOVE(13)
        case 14:
                                    ADD_DROP_MOVE(14)
        case 15:
                                      ADD_DROP_MOVE(15)
        case 16:
                                        ADD_DROP_MOVE(16)
        case 17:
                                          ADD_DROP_MOVE(17)
        case 18:
                                            ADD_DROP_MOVE(18)
        case 19:
                                              ADD_DROP_MOVE(19)
        case 20:
                                                ADD_DROP_MOVE(20)
        case 21:
                                                  ADD_DROP_MOVE(21)
        case 22:
                                                    ADD_DROP_MOVE(22)
        case 23:
                                                      ADD_DROP_MOVE(23)
        case 24:
                                                        ADD_DROP_MOVE(24)
        case 25:
                                                          ADD_DROP_MOVE(25)
        case 26:
                                                            ADD_DROP_MOVE(26)
        case 27:
                                                              ADD_DROP_MOVE(27)
        case 28:
                                                                ADD_DROP_MOVE(28)
        case 29:
                                                                  ADD_DROP_MOVE(29)
        case 30:
                                                                    ADD_DROP_MOVE(30)
        case 31:
                                                                      ADD_DROP_MOVE(31)
        case 32:
                                                                        ADD_DROP_MOVE(32)
        case 33:
                                                                          ADD_DROP_MOVE(33)
        case 34:
                                                                            ADD_DROP_MOVE(34)
        case 35:
                                                                              ADD_DROP_MOVE(35)
        case 36:
                                                                                ADD_DROP_MOVE(36)
        case 37:
                                                                                  ADD_DROP_MOVE(37)
        case 38:
                                                                                    ADD_DROP_MOVE(38)
        case 39:
                                                                                      ADD_DROP_MOVE(39)
        case 40:
                                                                                        ADD_DROP_MOVE(40)
        default:
                                                                                          assert(false);
      }
#undef ADD_DROP_MOVE
}

void gen_legals(List &list, const Pos &pos) {

}
void gen_moves(List &list, const Pos &pos) {

}
void gen_moves(List &list, const Pos &pos, const bit::Bitboard &checks) {

}
void gen_evasions(List &list, const Pos &pos, const bit::Bitboard &checks) {

}
void gen_captures(List &list, const Pos &pos) {

}