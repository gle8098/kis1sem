cmake_minimum_required(VERSION 3.19)
project(meta2_task1)

set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)

add_executable(meta2_task1 test.cpp typelist.hpp)
target_link_libraries(meta2_task1 GTest::gtest GTest::gtest_main)
