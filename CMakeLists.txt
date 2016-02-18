cmake_minimum_required(VERSION 3.3)
project(ezplane)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp bsp.cpp bsp.h main.h ezp.cpp ezp.h)
add_executable(ezplane ${SOURCE_FILES})