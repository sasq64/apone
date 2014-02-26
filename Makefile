UTILS=../utils
GRAPPIX=grappix
CHIPM=../chipmachine

CXX=clang++
CC=clang
#USE_CCACHE=1
#HOST=android
include $(UTILS)/config.mk

OBJDIR := obj/
TARGET := demo
CFLAGS += -Wall -g -O2
CXXFLAGS += -std=c++0x

include $(CHIPM)/src/plugins/ModPlugin/module.mk
include $(UTILS)/coreutils/module.mk
#include $(UTILS)/webutils/module.mk
include $(GRAPPIX)/module.mk

INCLUDES += $(CHIPM)/src

DATA_FILES += data/ObelixPro.ttf data/test.mod

ANDROID_PROJECT := android

ifeq ($(HOST),android)
  #ADK=/opt/arm-linux-androideabi
  #SDK=/opt/android-sdk-linux
  TARGET_EXT := .apk
  #LDFLAGS += --sysroot=/opt/android-ndk-r9/platforms/android-14/arch-arm
  #LOCAL_FILES += android/android_native_app_glue.c
  #CFLAGS += -Iandroid 
  #-I$(ADK)/include -I$(ADK)/include/freetype2
  #LIBS += $(ADK)/lib/libfreetype.a $(ADK)/lib/libpng.a -lz -llog -landroid -lEGL -lGLESv2
  #LDFLAGS += -lz -llog -landroid -lEGL -lGLESv2
  LDFLAGS += -lOpenSLES
   LOCAL_FILES += AudioPlayerSL.cpp
else ifeq ($(HOST),emscripten)
 # LDFLAGS += -s TOTAL_MEMORY=67108864
  # -s DISABLE_EXCEPTION_CATCHING=0
  # -s OUTLINING_LIMIT=30000
  #LDFLAGS += -s EXPORTED_FUNCTIONS="['_main', '_set_searchstring', '_play_index']"
  TARGETDIR := html/
  CFLAGS += -DMUSIC
  LOCAL_FILES += AudioPlayer.cpp MusicPlayer.cpp Fifo.cpp
else
  CFLAGS += -DMUSIC
  LOCAL_FILES += AudioPlayer.cpp MusicPlayer.cpp Fifo.cpp
  LOCAL_DIRS += fft
  CFLAGS += -pthread
  LDFLAGS += -pthread -lpthread
endif

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

include $(UTILS)/build.mk
