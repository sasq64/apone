UTILS=../utils
GRAPPIX=grappix
CXX=clang++
CC=clang
USE_CCACHE=1

include $(UTILS)/config.mk

OBJDIR := obj/
TARGET := demo
CFLAGS += -Wall -O2 -I. -I$(UTILS)
CXXFLAGS += -std=c++0x
CHIPM=../chipmachine

include $(UTILS)/coreutils/module.mk
include $(UTILS)/webutils/module.mk
include $(GRAPPIX)/module.mk
include $(CHIPM)/src/plugins/ModPlugin/module.mk

ifeq ($(HOST),android)
  ADK=/opt/arm-linux-androideabi
  SDK=/opt/android-sdk-linux
  APP_PLATFORM=android-10

  LDFLAGS += --sysroot=/opt/android-ndk-r9/platforms/android-14/arch-arm
  LOCAL_FILES += android/android_native_app_glue.c
  CFLAGS += -Iandroid -I$(ADK)/include -I$(ADK)/include/freetype2
  LIBS += $(ADK)/lib/libfreetype.a $(ADK)/lib/libpng.a -lz -llog -landroid -lEGL -lGLESv2
else ifeq ($(HOST),emscripten)
  LDFLAGS += --preload-file data/ObelixPro.ttf --preload-file data/test.mod -s TOTAL_MEMORY=67108864
  # --preload-file fonts -s OUTLINING_LIMIT=50000
  # 
  # -s DISABLE_EXCEPTION_CATCHING=0
#LDFLAGS += -g -s OUTLINING_LIMIT=30000
  #LDFLAGS += -L$(CHIPM)/src/plugins/VicePlugin/em -L$(CHIPM)/src/plugins/ModPlugin/em
  LDFLAGS += -s EXPORTED_FUNCTIONS="['_main', '_set_searchstring', '_play_index']"
  TARGETDIR := html/
else
  #LIBS += -lviceplugin -lmodplugin
  #LDFLAGS +=-L$(CHIPM)/src/plugins/ModPlugin -L$(CHIPM)/src/plugins/VicePlugin
endif

## Hack that lets us run the currently open file from Sublime if it is one of the main files
MAIN_FILES = demo.cpp snake.cpp tiletest2.cpp bobs.cpp simple.cpp blur.cpp map.cpp
MAIN_FILE := demo.cpp
ifneq ($(ACTIVEFILE),)
 ifneq ($(findstring $(ACTIVEFILE),$(MAIN_FILES)),)
  MAIN_FILE := $(ACTIVEFILE)
 endif
endif

#FILES += $(CHIPM)/src/sqlite3/sqlite3.c $(CHIPM)/src/SongDb.cpp $(CHIPM)/src/SearchIndex.cpp
#LIBS += -ldl
#O2
CFLAGS += -Iflatland -DMUSIC
LOCAL_FILES += $(MAIN_FILE)
# flatland/Primitive.cpp

include $(UTILS)/build.mk
