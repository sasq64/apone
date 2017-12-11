cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.image)

set(SOURCE_FILES image.cpp lodepng.cpp)
#packer.cpp)


#find_package(PNG REQUIRED)
#include_directories(${PNG_INCLUDE_DIRS})
#add_definitions(${PNG_DEFINITIONS})

add_library(image ${SOURCE_FILES})
#target_link_libraries(image ${PNG_LIBRARIES})
target_include_directories(image PRIVATE ..)
