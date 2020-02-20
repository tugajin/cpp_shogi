cmake_minimum_required(VERSION 3.10)
project(cpp_shogi CXX)
add_executable(shogi attack.cpp bit.cpp common.cpp eval.cpp gen.cpp hash.cpp libmy.cpp main.cpp math.cpp move.cpp hand.cpp pos.cpp score.cpp search.cpp sfen.cpp sort.cpp thread.cpp tt.cpp uct.cpp util.cpp var.cpp)
set(CMAKE_CXX_FLAGS "-pthread -ansi -pedantic -Wall -Wextra -std=c++14 -fno-rtti -g -mpopcnt -mbmi2 -msse4.2 -mavx2 -DBMI -DDEBUG -flto")

