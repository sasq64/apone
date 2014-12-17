cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.archive)

add_subdirectory(unrar)
add_subdirectory(ziplib)

set(SOURCE_FILES archive.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")


add_library(archive ${SOURCE_FILES})
target_link_libraries(archive unrar ziplib)