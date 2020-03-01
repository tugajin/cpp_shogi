#include "hash.hpp"
namespace hash {
	Key gKeyTurn;
	Key gKeyPiece[SIDE_SIZE][PIECE_SIZE][SQUARE_SIZE];
	Key gKeyHand[SIDE_SIZE][PIECE_SIZE][20];

	//https://ja.wikipedia.org/wiki/Xorshift
	static uint32 xor128() {
		static uint32_t x = 123456789;
		static uint32_t y = 362436069;
		static uint32_t z = 521288629;
		static uint32_t w = 88675123;
		uint32_t t;

		t = x ^ (x << 11);
		x = y; y = z; z = w;
		return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
	}
	Key get_key() {
		uint64 bi = xor128();
		uint64 li = xor128();
		return Key((bi << 32) | (li));
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