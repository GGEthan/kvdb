cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_FLAGS   "-std=c++11 -g -Wall")
 
PROJECT(kvdb)
AUX_SOURCE_DIRECTORY(../src DIR_SRCS)


ADD_EXECUTABLE(main ${DIR_SRCS})
