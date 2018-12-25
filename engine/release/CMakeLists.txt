cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_CXX_FLAGS   "-std=c++11 -g -pthread")
 
PROJECT(kvdb)

#it's depended
#include_directories("/usr/java/jdk1.8.0_181/include")
#include_directories("/usr/java/jdk1.8.0_181/include/linux")

AUX_SOURCE_DIRECTORY(.. DIR_SRCS)

ADD_LIBRARY(kvdb SHARED ${DIR_SRCS})
