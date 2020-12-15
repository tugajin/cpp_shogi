cmake_minimum_required(VERSION 3.12)
project(cpp_lib_project)

find_package(Python3 COMPONENTS Development)
find_package(Boost COMPONENTS python38 numpy38 REQUIRED)

add_library(cpp_lib SHARED libmain.cpp ../cpp/nn.cpp ../cpp/pos.cpp ../cpp/sfen.cpp ../cpp/move.cpp ../cpp/hand.cpp ../cpp/common.cpp ../cpp/bit.cpp ../cpp/hash.cpp ../cpp/libmy.cpp ../cpp/attack.cpp ../cpp/util.cpp ../cpp/gen.cpp ../cpp/eval.cpp)

set_property(TARGET cpp_lib PROPERTY CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-pthread -ansi -pedantic -Wall -Wextra -O3 -mpopcnt -mbmi2 -msse4.2 -mavx2 -DBMI -flto")

target_include_directories(cpp_lib PRIVATE ${Python3_INCLUDE_DIRS})
target_link_libraries(cpp_lib PRIVATE ${Boost_LIBRARIES} ${Python3_LIBRARIES})
set_target_properties(cpp_lib PROPERTIES PREFIX "") # 接頭辞'lib'を省略するため

