#include "hash.hpp"
namespace hash {
    Key gKeyTurn;
    Key gKeyPiece[SIDE_SIZE][PIECE_SIZE][SQUARE_SIZE];
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
        for(auto &k1 : gKeyPiece) {
            for(auto &k2 : k1) {
                for(auto &k3 : k2) {
                    k3 = get_key();
                }
            }
        }
    }
}