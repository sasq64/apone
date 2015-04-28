cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.archive)

add_subdirectory(unrar)
#add_subdirectory(ziplib)
if (NOT TARGET miniz)
	add_subdirectory(../miniz miniz)
endif()

set(SOURCE_FILES archive.cpp)


add_library(archive ${SOURCE_FILES})
target_link_libraries(archive unrar miniz)