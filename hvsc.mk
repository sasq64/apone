UTILS=../cpp-mods
CHIPM=../chipmachine
GRAPPIX=.

#CXX=clang++
#CC=clang
#USE_CCACHE=1

include $(UTILS)/config.mk

OBJDIR := obj/
TARGET := hvsc
CFLAGS += -Wall -O2 -I. -I$(UTILS)
CXXFLAGS += -std=c++0x

INCLUDES += $(CHIPM)/src

include $(UTILS)/coreutils/module.mk
include $(UTILS)/webutils/module.mk
include $(UTILS)/sqlite3/module.mk
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

include $(UTILS)/build.mk
