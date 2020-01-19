#include "bit.hpp"
#include <array>

namespace bit {

std::array<Bitboard, SQUARE_SIZE> g_mask;
std::array<Bitboard, SQUARE_SIZE> g_rook_mask;
std::array<Bitboard, SQUARE_SIZE> g_bishop_mask;
std::array<Bitboard, FILE_SIZE> g_file_mask;
std::array<Bitboard, RANK_SIZE> g_rank_mask;

std::array<Bitboard, SIDE_SIZE> g_prom; //1~3
std::array<Bitboard, SIDE_SIZE> g_middle; //4~9
Bitboard G_ALL_ONE_BB;
std::array<std::array<Bitboard, 1 << 9>, SIDE_SIZE> g_double_pawn_mask;

void init() {
    //mask init
    SQUARE_FOREACH(i){
      g_mask[i].init();
      (!bit::Bitboard::select(i)) ?
        g_mask[i].set(0, (1ull << i)) :
        g_mask[i].set(1, (1ull << (i - 63)));
    }
    //rank mask init
    //file_mask init
    for (auto &bb : g_file_mask) {
      bb.init();
    }
    for (auto &bb : g_rank_mask) {
      bb.init();
    }
    SQUARE_FOREACH(i){
      const auto file = square_file(i);
      const auto rank = square_rank(i);
      g_file_mask[file] |= i;
      g_rank_mask[rank] |= i;
    }
    //rook mask bishop mask int
    auto init_func = [](std::array<bit::Bitboard,SQUARE_SIZE> &bb_list,
        std::array<int,2>file_inc,
        std::array<int,2>rank_inc,
        bool is_bishop) {
      SQUARE_FOREACH(sq){
        auto file = square_file(sq);
        auto rank = square_rank(sq);
        for(auto f_inc : file_inc) {
          for(auto r_inc : rank_inc) {
            auto f = file;
            auto r = rank;
            while(true) {
              f += f_inc;
              r += r_inc;
              if(is_valid(f,r)) {
                bb_list[sq] |= square_make(f,r);
              } else {
                break;
              }
            }
          }
        }
        if(is_bishop) {
          bb_list[sq] &= ~g_file_mask[File_1];
          bb_list[sq] &= ~g_file_mask[File_9];
          bb_list[sq] &= ~g_rank_mask[Rank_1];
          bb_list[sq] &= ~g_rank_mask[Rank_9];
        } else {
          if(square_file(sq) != File_1) {
            bb_list[sq] &= ~g_file_mask[File_1];
          }
          if(square_file(sq) != File_9) {
            bb_list[sq] &= ~g_file_mask[File_9];
          }
          if(square_rank(sq) != Rank_1) {
            bb_list[sq] &= ~g_rank_mask[Rank_1];
          }
          if(square_rank(sq) != Rank_9) {
            bb_list[sq] &= ~g_rank_mask[Rank_9];
          }
        }
      }
    };
    for (auto &bb : g_rook_mask) {
        bb.init();
    }
    for (auto &bb : g_bishop_mask) {
        bb.init();
    }
    init_func(g_rook_mask, { 0, 0 }, { -1, 1 }, false);
    init_func(g_rook_mask, { -1, 1 }, { 0, 0 }, false);
    init_func(g_bishop_mask, { 1, 1 }, { -1, 1 }, true);
    init_func(g_bishop_mask, { -1, -1 }, { -1, 1 }, true);

    //prom middle
    g_prom[BLACK].init();
    g_prom[WHITE].init();
    g_middle[BLACK].init();
    g_middle[WHITE].init();
    SQUARE_FOREACH(sq){
      const auto rank = square_rank(sq);
      if (rank == Rank_1 || rank == Rank_2
          || rank == Rank_3) {
        g_prom[BLACK].set(sq);
      }
      if (rank == Rank_7 || rank == Rank_8
          || rank == Rank_9) {
        g_prom[WHITE].set(sq);
      }
      if ( rank == Rank_4 || rank == Rank_5
          || rank == Rank_6 || rank == Rank_7
          || rank == Rank_8 || rank == Rank_9) {
        g_middle[BLACK].set(sq);
      }
        if (rank == Rank_1 || rank == Rank_2
          || rank == Rank_3 || rank == Rank_4
          || rank == Rank_5 || rank == Rank_6) {
            g_middle[WHITE].set(sq);
        }
    }
    //double pawn mask
    for (auto index = 0; index < (1 << FILE_SIZE); index++) {
      g_double_pawn_mask[BLACK][index].init();
      g_double_pawn_mask[WHITE][index].init();
      for (auto shift = 0; shift < FILE_SIZE; shift++) {
            if (!(index & (1 << shift))) {
                g_double_pawn_mask[BLACK][index] |= g_file_mask[shift];
                g_double_pawn_mask[WHITE][index] |= g_file_mask[shift];
            }
        }
        g_double_pawn_mask[BLACK][index] &= ~g_rank_mask[Rank_1];
        g_double_pawn_mask[WHITE][index] &= ~g_rank_mask[Rank_9];
    }
    //all_one
    G_ALL_ONE_BB .init();
    SQUARE_FOREACH(sq){
        G_ALL_ONE_BB.set(sq);
    }
}

void test() {
    std::cout<<"test bit start\n";
    {
        auto bb = Bitboard();
        bb.init();
        Tee<<bb<<std::endl;
    }
    std::cout<<"test bit end\n";

}

}