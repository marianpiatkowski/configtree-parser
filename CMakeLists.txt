cmake_minimum_required(VERSION 3.2)

project(configtree-parser C CXX)

set(CMAKE_CXX_STANDARD 11)

add_compile_options(-Wall -O2)

enable_testing()

add_executable(configtreetest configtreetest.cc)
add_test(configtreetest configtreetest)
