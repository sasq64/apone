ifeq ($(LUA_INCLUDED),)
LUA_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

LUADIR := $(THIS_DIR)/lua-5.2.3/src/
lua_CFLAGS := -O2 -Wall -DLUA_COMPAT_ALL -DLUA_USE_POSIX -Wno-parentheses-equality
lua_INCLUDES += $(LUADIR)

lua_CC := g++
ifdef USE_CLANG
 ifneq ($(HOST),raspberrypi)
  lua_CC := clang++
 endif
endif


lua_FILES := $(THIS_DIR)/luainterpreter.cpp \
$(LUADIR)lapi.c \
$(LUADIR)lcode.c \
$(LUADIR)lctype.c \
$(LUADIR)ldebug.c \
$(LUADIR)ldo.c \
$(LUADIR)ldump.c \
$(LUADIR)lfunc.c \
$(LUADIR)lgc.c \
$(LUADIR)llex.c \
$(LUADIR)lmem.c \
$(LUADIR)lobject.c \
$(LUADIR)lopcodes.c \
$(LUADIR)lparser.c \
$(LUADIR)lstate.c \
$(LUADIR)lstring.c \
$(LUADIR)ltable.c \
$(LUADIR)ltm.c \
$(LUADIR)lundump.c \
$(LUADIR)lvm.c \
$(LUADIR)lzio.c \
$(LUADIR)lauxlib.c \
$(LUADIR)lbaselib.c \
$(LUADIR)lbitlib.c \
$(LUADIR)lcorolib.c \
$(LUADIR)ldblib.c \
$(LUADIR)liolib.c \
$(LUADIR)lmathlib.c \
$(LUADIR)loslib.c \
$(LUADIR)lstrlib.c \
$(LUADIR)ltablib.c \
$(LUADIR)loadlib.c \
$(LUADIR)linit.c

INCLUDES += $(THIS_DIR)..

MODULES += lua


endif