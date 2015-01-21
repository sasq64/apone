cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.sqlite3)


set(SOURCE_FILES database.cpp sqlite3.c)
add_definitions(-DSQLITE_ENABLE_FTS3 -DSQLITE_ENABLE_FTS3_PARENTHESIS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

if(UNIX)
  set(EXTRA_LIBS dl)
endif()


add_library(sqlite3 ${SOURCE_FILES})

target_link_libraries(sqlite3 pthread ${EXTRA_LIBS})
