cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.bbsutils)



add_subdirectory(../netlink netlink)
include_directories(../netlink)

set(SOURCE_FILES
	ansiconsole.cpp
	console.cpp
	editor.cpp
	localconsole.cpp
	petsciiconsole.cpp
	telnetserver.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")


add_library(bbsutils ${SOURCE_FILES})
target_link_libraries(bbsutils netlink)
