#ifndef BIT_HPP
#define BIT_HPP

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

#include "libmy.hpp"
#include "common.hpp"

#include <array>
#include <bitset>

namespace bit {

//Apprry、やねうら王モロパクリ

class Bitboard {
    private:
        union {
            uint64 p_[2];
            __m128i m_;
        };
    public:
        Bitboard() {}
        Bitboard(uint64 u0, uint64 u1) {
            this->set(u0,u1);
        }
        Bitboard &operator = (const Bitboard &rhs) {
            _mm_store_si128(&this->m_, rhs.m_);
            return *this;
        }
        Bitboard operator &=(const Bitboard& rhs) {
            this->m_ = _mm_and_si128(this->m_, rhs.m_);
            return *this; 
        }
        Bitboard operator |=(const Bitboard& rhs) {
            this->m_ = _mm_or_si128(this->m_, rhs.m_); 
            return *this;
        }
        Bitboard operator ^=(const Bitboard& rhs) {
            this->m_ = _mm_xor_si128(this->m_, rhs.m_); 
            return *this;
        }
        Bitboard operator <<=(const int i) {
            this->m_ = _mm_slli_epi64(this->m_, i);
            return *this;
        }
        Bitboard operator >>=(const int i) {
            this->m_ = _mm_srli_epi64(this->m_, i);
            return *this;
        }
        Bitboard operator &(const Bitboard& rhs) const {
            return Bitboard(*this) &= rhs;
        }
        Bitboard operator |(const Bitboard& rhs) const {
            return Bitboard(*this) |= rhs;
        }
        Bitboard operator ^(const Bitboard& rhs) const {
            return Bitboard(*this) ^= rhs;
        }
        Bitboard operator <<(const int i) const {
            return Bitboard(*this) <<= i;
        }
        Bitboard operator >>(const int i) const {
            return Bitboard(*this) >>= i;
        }
        bool operator ==(const Bitboard& rhs) const {
       	    __m128i neq = _mm_xor_si128(this->m_, rhs.m_);
	        return _mm_test_all_zeros(neq, neq) ? true : false;
        }
        bool operator !=(const Bitboard& rhs) const {
            return !(*this == rhs);
        }
        Bitboard& operator += (const Bitboard& b1) { 
            this->m_ = _mm_add_epi64(this->m_, b1.m_); 
            return *this; 
        } 
        template<int index>uint64 p() const {
            return (uint64)(_mm_extract_epi64(this->m_,index));
        }
        template<int index> void select_set(const uint64 u) {
            this->m_ = _mm_insert_epi64(this->m_,u,index);
        }
        void set(const uint64 u0, const uint64 u1) {
            this->m_ = _mm_set_epi64x(u1,u0);
        }
        void select_set(const int index, const uint64 u) {
            (!index) ? select_set<0>(u) : select_set<1>(u);
        }
        uint64 select_p(const int index) const {
            return (!index) ? p<0>() : p<1>();
        }
        void init() {
            set(0ull,0ull);
        }
        uint64 merge() const {
            return (p<0>() | p<1>());
        }
        explicit operator bool() const {
            return !_mm_testz_si128(this->m_,this->m_);
            //return !(_mm_testz_si128(this->m_, _mm_set1_epi8(static_cast<char>(0xffu))));
        }
        int pop_cnt() const {
            auto num = 0;
            if (p<0>()) {
                num += ml::bit_count(p<0>());
            }
            if (p<1>()) {
                num += ml::bit_count(p<1>());
            }
            return num;
        }
        template<bool is_del = true> Square lsb_right() {
            assert(bool(*this));
            const auto ret = (ml::bit_first(this->p<0>()));
            if (is_del) {
                p_[0] &= p_[0] - 1;
            }
            assert(ret >= 0 && ret <= 63);
            return Square(ret);
        }
        template<bool is_del = true> Square lsb_left() {
            assert(bool(*this));
            const auto ret = (ml::bit_first(this->p<1>()));
            if (is_del) {
                p_[1] &= p_[1] - 1;
            }
            assert(ret >= 0 && ret <= 17);
            return Square(ret + 63);
        }
        template<bool is_del = true> Square lsb() {
            assert(bool(*this));
            if (this->p<0>()) {
                return lsb_right<is_del>();
            }
            return lsb_left<is_del>();
        }
        friend std::ostream& operator<<(std::ostream& os, const Bitboard& b) {
            os << b.p<0>() << std::endl;
            os << b.p<1>() << std::endl;
            os << "-----------------" << std::endl;
            for (int rank = 0; rank < 9; rank++) {
                for (int file = 8; file >= 0; file--) {
                    int xy = rank + file * 9;
                    (b.is_set(xy)) ? os << "1," : os << "0,";
                }
                os << std::endl;
            }
            return os;
        }
        bool is_set(const int) const;
        Bitboard operator &=(const int);
        Bitboard operator |=(const int);
        Bitboard operator ^=(const int);
        Bitboard operator &(const int xy) const {
            return Bitboard(*this) &= xy;
        }
        Bitboard operator |(const int xy) const {
            return Bitboard(*this) |= xy;
        }
        Bitboard operator ^(const int xy) const {
            return Bitboard(*this) ^= xy;
        }
        Bitboard operator + (const Bitboard& rhs) const { 
            return Bitboard(*this) += rhs; 
        }
        void clear(const Square sq) {
            assert(is_set(sq));
            (*this) ^= sq;
        }
        void set(const Square sq) {
            (*this) |= sq;
        }
        constexpr static int select(const int xy) {
            return static_cast<int>(xy >= 63);
        }
};

extern std::array<Bitboard, SQUARE_SIZE> g_mask;
extern std::array<Bitboard, SQUARE_SIZE> g_diag1_mask;
extern std::array<Bitboard, SQUARE_SIZE> g_diag2_mask;
extern std::array<Bitboard, FILE_SIZE> g_file_mask;
extern std::array<Bitboard, RANK_SIZE> g_rank_mask;

extern std::array<Bitboard, SIDE_SIZE> g_prom; //1~3
extern std::array<Bitboard, SIDE_SIZE> g_middle; //4~9
extern Bitboard G_ALL_ONE_BB;
extern std::array<std::array<Bitboard, 1 << 9>, SIDE_SIZE> g_double_pawn_mask;
const std::array<int, SQUARE_SIZE> g_file_shift = { 
  1, 1, 1, 1, 1, 1, 1,1, 1, 
  10, 10, 10, 10, 10, 10, 10, 10, 10, 
  19, 19, 19, 19, 19, 19, 19,19, 19, 
  28, 28, 28, 28, 28, 28, 28, 28, 28, 
  37, 37, 37, 37, 37, 37, 37,37, 37, 
  46, 46, 46, 46, 46, 46, 46, 46, 46, 
  55, 55, 55, 55, 55, 55, 55,55, 55, 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 
  10, 10, 10, 10, 10, 10, 10, 10, 10, };

typedef std::array<std::array<bit::Bitboard, SQUARE_SIZE>, SIDE_SIZE> bw_square_t;

extern bw_square_t g_knight_attacks;
extern bw_square_t g_silver_attacks;
extern bw_square_t g_gold_attacks;
extern std::array<std::array<uint64,128>, FILE_SIZE> g_file_attack;
extern std::array<std::array<bit::Bitboard,128>, RANK_SIZE> g_rank_attack;
extern std::array<bit::Bitboard,1856+1> g_diag1_attack;
extern std::array<bit::Bitboard,1856+1> g_diag2_attack;
extern std::array<int,SQUARE_SIZE> g_diag1_offset;
extern std::array<int,SQUARE_SIZE> g_diag2_offset;

extern bw_square_t g_lance_mask;

extern Bitboard gBetween[638];
extern Bitboard gBeyond[393];

extern int gBetweenIndex[SQUARE_SIZE][SQUARE_SIZE];
extern int gBeyondIndex[SQUARE_SIZE][SQUARE_SIZE];

inline Bitboard Bitboard::operator &=(const int xy) {
    *this &= g_mask[xy];
    return *this;
}

inline Bitboard Bitboard::operator |=(const int xy) {
    *this |= g_mask[xy];
    return *this;
}

inline Bitboard Bitboard::operator ^=(const int xy) {
    *this ^= g_mask[xy];
    return *this;
}


inline bool Bitboard::is_set(const int sq) const {
    return (bool)((*this) & sq);
}

inline Bitboard operator ~ (const Bitboard& bb) { return bb ^ G_ALL_ONE_BB; }

inline uint64 occ_to_index(const bit::Bitboard &bb, const bit::Bitboard &mask) {
    return ml::pext(bb.merge(), mask.merge());
}

inline bit::Bitboard get_diag1_attack(const Square sq, const bit::Bitboard &occ) {
    return bit::g_diag1_attack[bit::g_diag1_offset[sq]
      + occ_to_index(occ & bit::g_diag1_mask[sq], bit::g_diag1_mask[sq])];
}
inline bit::Bitboard get_diag2_attack(const Square sq, const bit::Bitboard &occ) {
    return bit::g_diag2_attack[bit::g_diag2_offset[sq]
      + occ_to_index(occ & bit::g_diag2_mask[sq], bit::g_diag2_mask[sq])];
}
inline bit::Bitboard get_pseudo_diag1_attack(const Square sq) {
    return bit::g_diag1_attack[bit::g_diag1_offset[sq]];
}
inline bit::Bitboard get_pseudo_diag2_attack(const Square sq) {
    return bit::g_diag2_attack[bit::g_diag2_offset[sq]];
}
inline bit::Bitboard get_file_attack(const Square sq, const bit::Bitboard &occ) {
    if(sq <= SQ_79) {
        auto index = (occ.p<0>() >> bit::g_file_shift[sq]) & 0x7f;
        auto rank = square_rank(sq);
        return bit::Bitboard(bit::g_file_attack[rank][index]<<(g_file_shift[sq]-1),0ull);
    } else {
        auto index = (occ.p<1>() >> bit::g_file_shift[sq]) & 0x7f;
        auto rank = square_rank(sq);
        return bit::Bitboard(0ull,bit::g_file_attack[rank][index]<<(g_file_shift[sq]-1));
    }
}
inline bit::Bitboard get_pseudo_file_attack(const Square sq) {
    if(sq <= SQ_79) {
        auto rank = square_rank(sq);
        return bit::Bitboard(bit::g_file_attack[rank][0]<<(g_file_shift[sq]-1),0ull);
    } else {
        auto rank = square_rank(sq);
        return bit::Bitboard(0ull,bit::g_file_attack[rank][0]<<(g_file_shift[sq]-1));
    }
}

inline bit::Bitboard get_rank_attack(const Square sq, const bit::Bitboard &occ) {
    auto file = square_file(sq);
    auto rank = square_rank(sq);
    auto u = (occ.p<0>() >> 9) + (occ.p<1>() << 54);
    uint64 index = ml::pext(u,0b1000000001000000001000000001000000001000000001000000001<<rank);
    return (bit::g_rank_attack[file][index])<<rank;
}

inline bit::Bitboard get_pseudo_rank_attack(const Square sq) {
    auto file = square_file(sq);
    auto rank = square_rank(sq);
    return (bit::g_rank_attack[file][0])<<rank;
}
inline bit::Bitboard get_rook_attack(const Square sq, const bit::Bitboard &occ) {
    return get_rank_attack(sq,occ) | get_file_attack(sq,occ);
}
inline bit::Bitboard get_pseudo_rook_attack(const Square sq) {
    return get_pseudo_rank_attack(sq) | get_pseudo_file_attack(sq);
}
inline bit::Bitboard get_bishop_attack(const Square sq, const bit::Bitboard &occ) {
    return get_diag1_attack(sq,occ) | get_diag2_attack(sq,occ);
}
inline bit::Bitboard get_pseudo_bishop_attack(const Square sq) {
    return get_pseudo_diag1_attack(sq) | get_pseudo_diag2_attack(sq);
}
inline bit::Bitboard get_lance_attack(const Side sd, const Square sq, const bit::Bitboard &occ) {
    return get_file_attack(sq,occ) & bit::g_lance_mask[sd][sq];
}
inline bit::Bitboard get_pseudo_lance_attack(const Side sd, const Square sq) {
    return bit::g_lance_mask[sd][sq];
}
inline bit::Bitboard get_knight_attack(const Side sd, const Square sq) {
    return bit::g_knight_attacks[sd][sq];
}
inline bit::Bitboard get_silver_attack(const Side sd, const Square sq) {
    return bit::g_silver_attacks[sd][sq];
}
inline bit::Bitboard get_gold_attack(const Side sd, const Square sq) {
    return bit::g_gold_attacks[sd][sq];
}
inline bit::Bitboard get_pawn_attack(const Side sd, const Square sq) {
    //歩は数が多いから専用のテーブル用意したほうがいいかも
    return get_gold_attack(sd,sq) & bit::g_lance_mask[sd][sq];
}
template<Side sd> bit::Bitboard get_pawn_attack(const bit::Bitboard pawn) {
    return (sd == BLACK) ? pawn >> 1 : pawn << 1;
}
inline bit::Bitboard get_king_attack(const Square sq) {
    return get_gold_attack(BLACK,sq) | get_gold_attack(WHITE,sq);
}
inline bit::Bitboard get_prook_attack(const Square sq, const bit::Bitboard &occ) {
    return (get_rook_attack(sq, occ) | get_king_attack(sq));
}
inline bit::Bitboard get_pseudo_prook_attack(const Square sq) {
    return (get_pseudo_rook_attack(sq) | get_king_attack(sq));
}
inline bit::Bitboard get_pbishop_attack(const Square sq, const bit::Bitboard &occ) {
    return (get_bishop_attack(sq, occ) | get_king_attack(sq));
}
inline bit::Bitboard get_pseudo_pbishop_attack(const Square sq) {
    return (get_pseudo_bishop_attack(sq) | get_king_attack(sq));
}
inline bit::Bitboard get_plus_attack(const Square sq) {
    return get_gold_attack(BLACK, sq) & get_gold_attack(WHITE, sq);
}
inline bit::Bitboard get_x_attack(const Square sq) {
    return get_silver_attack(BLACK, sq) & get_silver_attack(WHITE, sq);
}

inline bool line_is_empty(const Square from, const Square to, const bit::Bitboard &bb) {
    return !(gBetween[gBetweenIndex[from][to]] & bb);
}

inline Bitboard between(const Square from, const Square to) {
    return gBetween[gBetweenIndex[from][to]];
}

inline Bitboard beyond(const Square from, const Square to) {
    return gBeyond[gBeyondIndex[from][to]];
}
template<Piece pc>Bitboard piece_pseudo_attacks(const Side sd, const Square from) {
    switch(pc) {
        case Pawn : return get_pawn_attack(sd,from);
        case Knight : return get_knight_attack(sd,from);
        case Silver: return get_silver_attack(sd,from);
        case Gold : 
        case Golds:
        case PPawn : 
        case PLance : 
        case PKnight : 
        case PSilver : return get_gold_attack(sd,from);
        case King : return get_king_attack(from);
        case Rook : return get_pseudo_rook_attack(from);
        case Bishop : return get_pseudo_bishop_attack(from);
        case PRook: return get_pseudo_prook_attack(from);
        case PBishop : return get_pseudo_pbishop_attack(from);
        case Lance : return get_pseudo_lance_attack(sd,from);
        default : return Bitboard(0ull,0ull);
    }
}
template<Piece pc>Bitboard piece_attacks(const Side sd, const Square from, const Bitboard & pieces) {
    switch(pc) {
        case Pawn : return get_pawn_attack(sd,from);
        case Knight : return get_knight_attack(sd,from);
        case Silver: return get_silver_attack(sd,from);
        case Gold : 
        case Golds:
        case PPawn : 
        case PLance : 
        case PKnight : 
        case PSilver : return get_gold_attack(sd,from);
        case King : return get_king_attack(from);
        case Rook : return get_rook_attack(from,pieces);
        case Bishop : return get_bishop_attack(from,pieces);
        case PRook: return get_prook_attack(from,pieces);
        case PBishop : return get_pbishop_attack(from,pieces);
        case Lance : return get_lance_attack(sd,from,pieces);
        default : return Bitboard(0ull,0ull);
    }
}
template<Piece pc>bool piece_pseudo_attack(const Side sd, const Square from, const Square to) {
    return piece_pseudo_attacks<pc>(sd,from).is_set(to);
}
template<Piece pc>bool piece_attack(const Side sd, const Square from, const Square to, const Bitboard & pieces) {
    return piece_pseudo_attack<pc>(sd,from,to) && line_is_empty(from,to,pieces);
}


void init();
void test();

}



#endif