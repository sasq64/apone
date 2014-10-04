cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.coreutils)


set(SOURCE_FILES file.cpp format.cpp log.cpp utils.cpp)
add_definitions(-std=c++0x -O2)

add_library(coreutils ${SOURCE_FILES})
