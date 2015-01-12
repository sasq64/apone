cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.coreutils)


set(SOURCE_FILES file.cpp format.cpp log.cpp utils.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_library(coreutils ${SOURCE_FILES})

if(DO_UNIT_TEST)
add_executable(coreutils_test ../test/catch.cpp)
target_link_libraries(coreutils_test coreutils)
endif()