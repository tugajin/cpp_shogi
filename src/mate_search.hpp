#ifndef MATE_SEARCH
#define MATE_SEARCH

#include "common.hpp"

class Pos;

Move mate_search(const Pos &pos, const Ply ply);

namespace mate {
    void test();
}

#endif