cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.bbsutils)


file(GLOB SOURCE_FILES *.cpp)
add_definitions(-std=c++0x)

add_library(bbsutils ${SOURCE_FILES})

add_subdirectory(../netlink netlink)
include_directories(../netlink)
target_link_libraries(bbsutils netlink)
