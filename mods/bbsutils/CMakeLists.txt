cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.bbsutils)



add_subdirectory(../netlink netlink)
include_directories(../netlink)

set(SOURCE_FILES
	ansiconsole.cpp
	console.cpp
	editor.cpp
	petsciiconsole.cpp
	#telnetserver.cpp
)

if(NOT WINDOWS)
  set(SOURCE_FILES ${SOURCE_FILES} localconsole.cpp)
endif()




add_library(bbsutils ${SOURCE_FILES})
target_link_libraries(bbsutils netlink)
