cmake_minimum_required(VERSION 2.8.4)
project(examples)

add_definitions(-DAPP_NAME=examples)

set(CPP_MODS ../mods)

include_directories(${CPP_MODS})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g")

set(APONE_MODULES
	coreutils
	audioplayer
	grappix
	archive
)
add_subdirectory(${CPP_MODS} cppmods)
add_subdirectory(${CPP_MODS}/musicplayer/plugins/openmptplugin openmptplugin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

include(../cmake/Utils.cmake)
SET_DATA_FILES(data/stardust.mod data/ObelixPro.ttf)

add_executable(simple simple.cpp)
target_link_libraries(simple coreutils grappix)

add_executable(demo demo.cpp)
target_link_libraries(demo coreutils audioplayer grappix openmptplugin)

add_executable(snake snake.cpp)
target_link_libraries(snake coreutils grappix)

