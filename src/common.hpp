#ifndef COMMON_HPP
#define COMMON_HPP

#include "libmy.hpp"
#include "overloadEnumOperators.hpp"

constexpr int FILE_SIZE { 9 };
constexpr int RANK_SIZE { 9 };
constexpr int SQUARE_SIZE { FILE_SIZE * RANK_SIZE };
constexpr int SIDE_SIZE { 2 };
constexpr int PIECE_SIZE { 16 };
constexpr int PIECE_SIDE_SIZE { 32 }; // excludes Empty #

// types

enum Square : int { 
    SQ_11 = 0, SQ_12, SQ_13, SQ_14, SQ_15, SQ_16, SQ_17, SQ_18, SQ_19,
    SQ_21, SQ_22, SQ_23, SQ_24, SQ_25, SQ_26, SQ_27, SQ_28, SQ_29,
    SQ_31, SQ_32, SQ_33, SQ_34, SQ_35, SQ_36, SQ_37, SQ_38, SQ_39,
    SQ_41, SQ_42, SQ_43, SQ_44, SQ_45, SQ_46, SQ_47, SQ_48, SQ_49,
    SQ_51, SQ_52, SQ_53, SQ_54, SQ_55, SQ_56, SQ_57, SQ_58, SQ_59,
    SQ_61, SQ_62, SQ_63, SQ_64, SQ_65, SQ_66, SQ_67, SQ_68, SQ_69,
    SQ_71, SQ_72, SQ_73, SQ_74, SQ_75, SQ_76, SQ_77, SQ_78, SQ_79,
    SQ_81, SQ_82, SQ_83, SQ_84, SQ_85, SQ_86, SQ_87, SQ_88, SQ_89,
    SQ_91, SQ_92, SQ_93, SQ_94, SQ_95, SQ_96, SQ_97, SQ_98, SQ_99, SQ_NONE = -1 };
enum File   : int { File_1, File_2, File_3, File_4, File_5, File_6, File_7, File_8, File_9 };
enum Rank   : int { Rank_1, Rank_2, Rank_3, Rank_4, Rank_5, Rank_6, Rank_7, Rank_8, Rank_9 };
enum Side   : int { BLACK, WHITE};

enum Inc : int {
    Inc_N  = -1,
    Inc_S  =  1,
    Inc_E  = -FILE_SIZE,
    Inc_W  =  FILE_SIZE,
    Inc_SW =  Inc_S + Inc_W,
    Inc_NW =  Inc_N + Inc_W,
    Inc_NE =  Inc_N + Inc_E,
    Inc_SE =  Inc_S + Inc_E,
};

OverloadEnumOperators(Square);
OverloadEnumOperators(File);
OverloadEnumOperators(Rank);
OverloadEnumOperators(Side);

enum class Key : uint64;
enum class Move : uint32;


inline constexpr Square square_make(const File f, const Rank r) {
    return Square(r + f * FILE_SIZE);
}

inline constexpr Rank square_rank(const Square sq) {
    return Rank(sq % RANK_SIZE);
}

inline constexpr File square_file(const Square sq) {
    return File(sq / FILE_SIZE);
}

inline constexpr bool is_valid_file(const int f) {
    return (f >= File_1 && f < FILE_SIZE);
}
inline constexpr bool is_valid_rank(const int r) {
    return (r >= Rank_1 && r < RANK_SIZE);
}
inline constexpr bool is_valid(const int f, const int r) {
    return (is_valid_rank(r)) && (is_valid_file(f));
}
inline constexpr bool is_valid_sq(const int sq) {
    return (sq >= 0) && (sq < SQUARE_SIZE);
}

#define SQUARE_FOREACH(sq) for(auto sq = SQ_11; sq < SQUARE_SIZE; ++sq) 
#define SIDE_FOREACH(sd) for(auto sd = BLACK; sd < SIDE_SIZE; ++sd)
#endif