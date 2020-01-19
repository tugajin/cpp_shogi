#ifndef SFEN_HPP
#define SFEN_HPP

#include "pos.hpp"

const std::string START_SFEN = "lnsgkgsnl/1r5b1/ppppppppp/9/9/9/PPPPPPPPP/1B5R1/LNSGKGSNL b - 1";

Pos pos_from_sfen(std::string &sfen);

#endif