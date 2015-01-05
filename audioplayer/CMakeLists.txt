cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.audioplayer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")


set(SOURCE_FILES audioplayer.cpp)

if(APPLE)
	find_package(SDL)
	set(EXTRA_LIBS ${EXTRA_LIBS} ${SDL_LIBRARY})
	include_directories(${SDL_INCLUDE_DIR})
	add_definitions(-DSDL_AUDIO)
	set(SOURCE_FILES ${SOURCE_FILES} player_sdl.cpp)
endif()

add_library(audioplayer ${SOURCE_FILES})
target_link_libraries(audioplayer ${EXTRA_LIBS})
