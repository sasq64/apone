cmake_minimum_required(VERSION 2.8.4)
project(examples)

add_definitions(-DAPP_NAME=examples)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
# set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DCM_DEBUG")
# set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_RELEASE} -DCM_DEBUG")

set(CPP_MODS ../mods)

include_directories(${CPP_MODS})

file(GLOB SOURCE_FILES src/*.cpp)

set(CPP_MODS_MODULES
	coreutils
	audioplayer
	grappix
	bbsutils
	fft
	archive
	webutils
	json
	luainterpreter
	sqlite3
)
add_subdirectory(${CPP_MODS} cppmods)

add_subdirectory(${CPP_MODS}/musicplayer/plugins/modplugin modplugin)

include_directories(${CPP_MODS}/grappix)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++0x")

add_executable(simple simple.cpp)
target_link_libraries(simple coreutils grappix)

add_executable(demo demo.cpp)
target_link_libraries(demo coreutils audioplayer grappix modplugin)

add_executable(snake snake.cpp)
target_link_libraries(snake coreutils grappix)

#add_executable(demo demo.cpp)
#target_link_libraries(demo ${CPP_MODS_MODULES})
