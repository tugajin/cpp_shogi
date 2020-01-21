#ifndef BIT_HPP
#define BIT_HPP

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

#include "libmy.hpp"
#include "common.hpp"

#include <array>

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
            this->m_ = _mm_slli_epi64(this->m_, i);
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
        template<bool is_del = true> int lsb_right() {
            assert(!is_empty());
            const auto ret = (ml::bit_first(this->p<0>()));
            if (is_del) {
                p_[0] &= p_[0] - 1;
            }
            assert(ret >= 0 && ret <= 63);
            return ret;
        }
        template<bool is_del = true> int lsb_left() {
            assert(!is_empty());
            const auto ret = (ml::bit_first(this->p<1>()));
            if (is_del) {
                p_[1] &= p_[1] - 1;
            }
            assert(ret >= 0 && ret <= 17);
            return (ret + 63);
        }
        template<bool is_del = true> int lsb() {
            assert(!is_empty());
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
extern std::array<Bitboard, SQUARE_SIZE> g_file_mask;
extern std::array<Bitboard, SQUARE_SIZE> g_rank_mask;

extern std::array<Bitboard, SIDE_SIZE> g_prom; //1~3
extern std::array<Bitboard, SIDE_SIZE> g_middle; //4~9
extern Bitboard G_ALL_ONE_BB;
extern std::array<std::array<Bitboard, 1 << 9>, SIDE_SIZE> g_double_pawn_mask;

typedef std::array<std::array<bit::Bitboard, SQUARE_SIZE>, SIDE_SIZE> bw_square_t;

extern bw_square_t g_knight_attacks;
extern bw_square_t g_silver_attacks;
extern bw_square_t g_gold_attacks;
extern std::array<std::array<bit::Bitboard,128>, SQUARE_SIZE> g_file_attack;
extern std::array<std::array<bit::Bitboard,128>, SQUARE_SIZE> g_rank_attack;
extern std::array<std::array<bit::Bitboard,128>, SQUARE_SIZE> g_diag1_attack;
extern std::array<std::array<bit::Bitboard,128>, SQUARE_SIZE> g_diag2_attack;

extern bw_square_t g_lance_mask;

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

  inline bit::Bitboard index_to_occ(const int index, const int bits,
      const bit::Bitboard &bb) {
    bit::Bitboard ret_bb;
    bit::Bitboard mask = bb;
    ret_bb.init();

    assert(bits == mask.pop_cnt());
    for (auto i = 0; i < bits; i++) {
      const auto sq = mask.lsb();
      if (index & (1 << i)) {
        ret_bb |= sq;
      }
    }
    return ret_bb;
  }
  inline uint64 occ_to_index(const bit::Bitboard &bb, const bit::Bitboard &mask) {
    return ml::pext(bb.merge(), mask.merge());
  }

void init();
void test();

}



#endif