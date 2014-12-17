cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.image)

set(SOURCE_FILES image.cpp packer.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")



find_package(PNG)
include_directories(${PNG_INCLUDE_DIRS})
add_definitions(${PNG_DEFINITIONS})

add_library(image ${SOURCE_FILES})
target_link_libraries(image ${PNG_LIBRARIES})