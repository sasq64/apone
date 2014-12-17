cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.json-c)

set(SOURCE_FILES
	arraylist.c
	debug.c
	json_c_version.c
	json_object.c
	json_object_iterator.c
	json_tokener.c
	json_util.c
	libjson.c
	linkhash.c
	printbuf.c
)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wwrite-strings -Wno-unused-parameter -std=gnu99 -fPIC")

add_definitions(-DHAVE_CONFIG_H -D_GNU_SOURCE -D_REENTRANT -fPIC -DPIC)

add_library(json-c ${SOURCE_FILES})
