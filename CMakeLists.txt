cmake_minimum_required(VERSION 3.28)
project(SimplePython)

set(CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")

add_executable(SimplePython main.cpp
        lexer.h
        lexer.cpp
        test_runner_p.h
        lexer_test_open.cpp)
