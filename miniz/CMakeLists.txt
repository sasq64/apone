cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.miniz)

set(SOURCE_FILES miniz.c)
#add_definitions(-Dxxx)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
#include_directories(.)

add_library(miniz ${SOURCE_FILES})
#target_link_libraries(sqlite3 xxx)
