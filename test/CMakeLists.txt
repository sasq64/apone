cmake_minimum_required(VERSION 2.8.4)
project(apone.test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2")

add_definitions(-DAPP_NAME=aponetest)

enable_testing()

#set(AP1_ROOT ..)

set(DO_UNIT_TEST 1)

set(APONE_MODULES
	coreutils
)
add_subdirectory(../mods ap1mods)
