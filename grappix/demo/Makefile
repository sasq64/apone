CPP_MODS=../../cpp-mods
GRAPPIX=..
CHIPM=../../chipmachine
include $(CPP_MODS)/config.mk

OBJDIR := obj/
TARGET := demo
CFLAGS += -Wall -g -O2 -DMUSIC

include $(CHIPM)/src/plugins/ModPlugin/module.mk
include $(CPP_MODS)/coreutils/module.mk
include $(CPP_MODS)/audioplayer/module.mk
include $(GRAPPIX)/module.mk

CC = clang
CXX = clang++

DATA_FILES += data/ObelixPro.ttf data/test.mod

ANDROID_PROJECT := android

ifeq ($(HOST),emscripten)
  # LDFLAGS += -s TOTAL_MEMORY=67108864 -s DISABLE_EXCEPTION_CATCHING=0 -s OUTLINING_LIMIT=30000
  # LDFLAGS += -s EXPORTED_FUNCTIONS="['_main', '_set_searchstring', '_play_index']"
  LDFLAGS += -s DISABLE_EXCEPTION_CATCHING=2
  TARGETDIR := html/
endif

LOCAL_FILES := MusicPlayer.cpp

## Hack that lets us run the currently open file from Sublime if it is one of the main files
MAIN_FILES = demo.cpp bobs.cpp snake.cpp tiletest2.cpp simple.cpp blur.cpp map.cpp
MAIN_FILE := $(firstword $(MAIN_FILES))
ifneq ($(ACTIVEFILE),)
 ifneq ($(findstring $(ACTIVEFILE),$(MAIN_FILES)),)
  MAIN_FILE := $(ACTIVEFILE)
 endif
endif

CFLAGS += -DMODPLUG_BASIC_SUPPORT
LOCAL_FILES += $(MAIN_FILE)

include $(CPP_MODS)/build.mk
