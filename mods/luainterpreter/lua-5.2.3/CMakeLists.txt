cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.lua)

#lua_CFLAGS := -O2 -Wall  -Wno-parentheses-equality
#set(CMAKE_C_COMPILER g++)

add_definitions(-DLUA_COMPAT_ALL -DLUA_USE_MKSTEMP)
# -DLUA_USE_POSIX)

set(SOURCE_FILES
	src/lapi.c
	src/lcode.c
	src/lctype.c
	src/ldebug.c
	src/ldo.c
	src/ldump.c
	src/lfunc.c
	src/lgc.c
	src/llex.c
	src/lmem.c
	src/lobject.c
	src/lopcodes.c
	src/lparser.c
	src/lstate.c
	src/lstring.c
	src/ltable.c
	src/ltm.c
	src/lundump.c
	src/lvm.c
	src/lzio.c
	src/lauxlib.c
	src/lbaselib.c
	src/lbitlib.c
	src/lcorolib.c
	src/ldblib.c
	src/liolib.c
	src/lmathlib.c
	src/loslib.c
	src/lstrlib.c
	src/ltablib.c
	src/loadlib.c
	src/linit.c
)

add_library(lua ${SOURCE_FILES})
