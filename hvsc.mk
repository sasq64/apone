CPP_MODS=../cpp-mods
CHIPM=../chipmachine
GRAPPIX=.

#CXX=clang++
#CC=clang
#USE_CCACHE=1

include $(CPP_MODS)/config.mk

OBJDIR := obj/
TARGET := hvsc
CFLAGS += -Wall -O2 -I.
CXXFLAGS += -std=c++0x

INCLUDES += $(CHIPM)/src

include $(CPP_MODS)/coreutils/module.mk
include $(CPP_MODS)/webutils/module.mk
include $(CPP_MODS)/sqlite3/module.mk
include $(CPP_MODS)/audioplayer/module.mk
include $(GRAPPIX)/module.mk
include $(CHIPM)/src/plugins/VicePlugin/module.mk

DATA_FILES += data/hvsc.db data/c64


ifeq ($(HOST),android)
  ANDROID_PROJECT := android
  TARGET_EXT := .apk
else ifeq ($(HOST),emscripten)
  LDFLAGS += -s TOTAL_MEMORY=67108864
  # -s DISABLE_EXCEPTION_CATCHING=0
  #LDFLAGS += -g -s OUTLINING_LIMIT=30000
  LDFLAGS += -s EXPORTED_FUNCTIONS="['_main', '_set_searchstring', '_play_index']"
  TARGETDIR := html/
endif

FILES += $(CHIPM)/src/SongDb.cpp $(CHIPM)/src/SearchIndex.cpp
LIBS += -ldl
LOCAL_FILES += sidplayer.cpp

include $(CPP_MODS)/build.mk
