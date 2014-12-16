cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.webutils)

set(SOURCE_FILES webgetter.cpp webrpc.cpp)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++0x)

add_library(webutils ${SOURCE_FILES})
target_link_libraries(webutils pthread curl)