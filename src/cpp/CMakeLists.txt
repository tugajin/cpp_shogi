cmake_minimum_required(VERSION 3.10)
project(cpp_shogi CXX)
set(CMAKE_PREFIX_PATH "/home/tugajin/libtorch")
#target_link_libraries(cpp_shogi "pthread")

find_package(Python3 COMPONENTS Development)
find_package(Boost COMPONENTS python38 numpy38 REQUIRED)

add_executable(cpp_shogi attack.cpp bit.cpp common.cpp eval.cpp gen.cpp hash.cpp libmy.cpp main.cpp mate_search.cpp math.cpp move.cpp nn.cpp hand.cpp pos.cpp score.cpp search.cpp sfen.cpp sort.cpp selfplay.cpp thread.cpp tt.cpp uct.cpp util.cpp var.cpp)

target_include_directories(cpp_shogi PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(cpp_shogi PRIVATE ${Boost_LIBRARIES} ${Python3_LIBRARIES})

set_property(TARGET cpp_shogi PROPERTY CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-pthread -ansi -pedantic -Wall -Wextra -O3 -mpopcnt -mbmi2 -msse4.2 -mavx2 -DBMI -DDEBUG -flto")
#set(CMAKE_CXX_FLAGS "-pthread -ansi -pedantic -Wall -Wextra -O3 -mpopcnt -mbmi2 -msse4.2 -mavx2 -DBMI -flto")
