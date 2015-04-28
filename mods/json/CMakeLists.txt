cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.json)

add_subdirectory(json-c)

set(SOURCE_FILES json.cpp)





add_library(json ${SOURCE_FILES})
target_link_libraries(json json-c)