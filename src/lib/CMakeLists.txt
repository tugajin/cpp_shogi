cmake_minimum_required(VERSION 3.12)
project(boost-python-test)

find_package(Python3 COMPONENTS Development)
find_package(Boost COMPONENTS python38 numpy38 REQUIRED)

add_executable(lib nn.cpp ../cpp/pos.cpp ../cpp/sfen.cpp ../cpp/move.cpp ../cpp/hand.cpp ../cpp/common.cpp ../cpp/bit.cpp ../cpp/hash.cpp ../cpp/libmy.cpp ../cpp/attack.cpp ../cpp/util.cpp ../cpp/gen.cpp ../cpp/eval.cpp)

set_property(TARGET lib PROPERTY CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-pthread -ansi -pedantic -Wall -Wextra -O3 -mpopcnt -mbmi2 -msse4.2 -mavx2 -DBMI -flto")

target_include_directories(lib PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(lib PRIVATE ${Boost_LIBRARIES} ${Python3_LIBRARIES})
