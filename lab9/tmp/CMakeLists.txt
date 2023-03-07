cmake_minimum_required(VERSION 3.15)
project(tmp C)

set(CMAKE_C_STANDARD 99)

SET(CMAKE_C_FLAGS -pthread)
find_package(Threads)

add_executable(tmp main.c)