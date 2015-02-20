cmake_minimum_required(VERSION 2.8.4)
project(skeleton)

## NOTE: YOU  MUST SET AP1_ROOT
set(AP1_ROOT $ENV{AP1_ROOT})

include(${AP1_ROOT}/cmake/Utils.cmake)

add_definitions(-DAPP_NAME=skeleton)

include_directories(${AP1_ROOT}/mods)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -g")

set(APONE_MODULES
	coreutils
	grappix
)
add_subdirectory(${AP1_ROOT}/mods ap1mods)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_executable(skeleton main.cpp)
target_link_libraries(skeleton coreutils grappix)
