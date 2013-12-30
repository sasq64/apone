UTILS=../utils
CHIPM=../chipmachine
GRAPPIX=grappix

#CXX=clang++
#CC=clang
#USE_CCACHE=1

include $(UTILS)/config.mk

OBJDIR := obj/
TARGET := hvsc
CFLAGS += -Wall -O2 -I. -I$(UTILS)
CXXFLAGS += -std=c++0x

include $(UTILS)/coreutils/module.mk
include $(UTILS)/webutils/module.mk
include $(GRAPPIX)/module.mk
include $(CHIPM)/src/plugins/VicePlugin/module.mk

DATA_FILES += data/hvsc.db data/c64

ifeq ($(HOST),android)
  ADK=/opt/arm-linux-androideabi
  SDK=/opt/android-sdk-linux
  APP_PLATFORM=android-10

  LDFLAGS += --sysroot=/opt/android-ndk-r9/platforms/android-14/arch-arm
  LOCAL_FILES += android/android_native_app_glue.c
  CFLAGS += -Iandroid -I$(ADK)/include -I$(ADK)/include/freetype2
  LIBS += $(ADK)/lib/libfreetype.a $(ADK)/lib/libpng.a -lz -llog -landroid -lEGL -lGLESv2
else ifeq ($(HOST),emscripten)
  LDFLAGS += -s TOTAL_MEMORY=67108864
  # -s DISABLE_EXCEPTION_CATCHING=0
  #LDFLAGS += -g -s OUTLINING_LIMIT=30000
  LDFLAGS += -s EXPORTED_FUNCTIONS="['_main', '_set_searchstring', '_play_index']"
  TARGETDIR := html/
endif

FILES += $(CHIPM)/src/sqlite3/sqlite3.c $(CHIPM)/src/SongDb.cpp $(CHIPM)/src/SearchIndex.cpp
LIBS += -ldl
LOCAL_FILES += sidplayer.cpp

include $(UTILS)/build.mk
