cmake_minimum_required(VERSION 3.16)
project(OpSys_Ex4 C)

set(CMAKE_C_STANDARD 90)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

add_executable(OpSys_Ex4 test.c osqueue.c osqueue.h threadPool.c threadPool.h)
