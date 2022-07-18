cmake_minimum_required(VERSION 3.21)
project(ffmpegParser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(ffmpegParser
        lib/src/parser.cpp lib/src/graph_checker.cpp lib/src/graph_convert.cpp parser_cli.cpp)

add_executable(test doctest_main.cpp
        parser_test.cpp)
