#ifndef MOVE_HPP
#define MOVE_HPP

#include <string>
#include "libmy.hpp"
#include "common.hpp"

class Pos;

namespace move {

Move from_usi(const std::string &s, const Pos &pos);

}

#endif