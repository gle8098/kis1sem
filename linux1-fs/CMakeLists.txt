cmake_minimum_required(VERSION 3.0)
project(linux1_fs C)

set(CMAKE_C_STANDARD 11)
set(COMPILE_OPTIONS ${COMPILE_OPTIONS} -g)

add_executable(linux1_fs main.c fs.h)
