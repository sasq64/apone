cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.bbsutils)

add_subdirectory(../netlink netlink)
#add_subdirectory(../coreutils coreutils)
include_directories(../netlink ..)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

set(SOURCE_FILES
	ansiconsole.cpp
	console.cpp
	editor.cpp
	petsciiconsole.cpp
	telnetserver.cpp
)

if(NOT WIN32)
  set(SOURCE_FILES ${SOURCE_FILES} localconsole.cpp)
endif()

add_library(bbsutils ${SOURCE_FILES})
target_link_libraries(bbsutils netlink coreutils)

#add_executable(bbstest test.cpp)
#target_link_libraries(bbstest bbsutils coreutils)
