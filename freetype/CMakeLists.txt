cmake_minimum_required(VERSION 2.8.4)
project(freetype)

add_subdirectory(freetype-2.5.5)
set(FREETYPE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/freetype-2.5.5/include PARENT_SCOPE)
message("FREETYPE ${FREETYPE_INCLUDE_DIR}")
