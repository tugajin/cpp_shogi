#include "hash.hpp"
namespace hash {
    Key gKeyTurn;
    Key gKeyPiece[SIDE_SIZE][PIECE_SIZE][SQUARE_SIZE];
    Key gKeyHand[SIDE_SIZE][PIECE_SIZE][20];
    
    //https://ja.wikipedia.org/wiki/Xorshift
    uint32 xor32() {
        static uint32_t y = 2463534242;
        y = y ^ (y << 13); y = y ^ (y >> 17);
        return y = y ^ (y << 5);
    }
    Key get_key() {
        uint64 bi = xor32();
        uint64 li = xor32();
        return  Key((bi << 32)| (li));
    }
    void init() {
        gKeyTurn = get_key();
        for (auto sd = 0; sd < SIDE_SIZE; sd++) {
            for (auto pc = 0; pc < PIECE_SIZE; pc++) {
                for (auto sq = 0; sq < SQUARE_SIZE; sq++) {
                    gKeyPiece[sd][pc][sq] = get_key();
                }
                for (auto i = 0; i < 20; i++) {
                    gKeyHand[sd][pc][i] = get_key();
                }
            }
        }
    }
}