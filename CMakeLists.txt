cmake_minimum_required(VERSION 3.3)
project(ezplane)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#set(SOURCE_FILES train.cpp bsp.cpp bsp.h train.h ezp.cpp ezp.h)

set(SOURCE_FILES predict.cpp predict.h bsp.cpp bsp.h ezp.cpp ezp.h)
add_executable(ezplane ${SOURCE_FILES})