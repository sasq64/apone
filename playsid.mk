UTILS=../utils
GRAPPIX=grappix
CXX=clang++
CC=clang
USE_CCACHE=1

include $(UTILS)/config.mk

OBJDIR := obj/
TARGET := playsid
#TARGET_EXT := .js
CFLAGS += -Wall -O2 -I. -I$(UTILS)
CXXFLAGS += -std=c++0x
CHIPM=../chipmachine

include $(UTILS)/coreutils/module.mk
#include $(UTILS)/webutils/module.mk

include $(CHIPM)/src/plugins/VicePlugin/module.mk

LDFLAGS += --post-js post.js -s RESERVED_FUNCTION_POINTERS=10 -s TOTAL_MEMORY=33554432 -s EXPORTED_FUNCTIONS="['_playsid_play','_playsid_pause','_playsid_setsong']"
TARGETDIR := html/
#LIBS += -ldl
#O2
#CFLAGS += -Iflatland -DMUSIC
#LOCAL_FILES += $(MAIN_FILE)
# flatland/Primitive.cpp
LOCAL_FILES = playsid.cpp

include $(UTILS)/build.mk
