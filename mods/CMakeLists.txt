cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods)

foreach(MODULE ${APONE_MODULES})
    add_subdirectory(${MODULE})
endforeach()