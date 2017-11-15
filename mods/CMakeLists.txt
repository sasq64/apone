cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

foreach(MODULE ${APONE_MODULES})
    add_subdirectory(${MODULE})
endforeach()