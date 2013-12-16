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
LDFLAGS += -g
CHIPM=../chipmachine

CFLAGS += -I$(CHIPM)/src -I$(CHIPM)/src/plugins/ModPlugin -I$(CHIPM)/src/plugins/VicePlugin

include $(UTILS)/coreutils/module.mk
include $(GRAPPIX)/module.mk


ifeq ($(HOST),android)
  ADK=/opt/arm-linux-androideabi
  SDK=/opt/android-sdk-linux
  APP_PLATFORM=android-10

  LDFLAGS += --sysroot=/opt/android-ndk-r9/platforms/android-14/arch-arm
  LOCAL_FILES += android/android_native_app_glue.c
  CFLAGS += -Iandroid -I$(ADK)/include -I$(ADK)/include/freetype2
  LIBS += $(ADK)/lib/libfreetype.a $(ADK)/lib/libpng.a -lz -llog -landroid -lEGL -lGLESv2
else ifeq ($(HOST),emscripten)
  LDFLAGS += --preload-file data
  # --preload-file fonts -s OUTLINING_LIMIT=50000
  # -s TOTAL_MEMORY=33554432
  # -s DISABLE_EXCEPTION_CATCHING=0
  LDFLAGS += -L$(CHIPM)/src/plugins/VicePlugin/em -L$(CHIPM)/src/plugins/ModPlugin/em
  TARGETDIR := html/
else
  LIBS += -lviceplugin -lmodplugin
  LDFLAGS +=-L$(CHIPM)/src/plugins/ModPlugin -L$(CHIPM)/src/plugins/VicePlugin
endif

## Hack that lets us run the currently open file from Sublime if it is one of the main files
MAIN_FILES = sidplayer.cpp demo.cpp snake.cpp tiletest2.cpp bobs.cpp simple.cpp blur.cpp map.cpp
MAIN_FILE := demo.cpp
ifneq ($(ACTIVEFILE),)
 ifneq ($(findstring $(ACTIVEFILE),$(MAIN_FILES)),)
  MAIN_FILE := $(ACTIVEFILE)
 endif
endif

LIBS += -lmodplugin

CFLAGS += -Iflatland -DMUSIC
LOCAL_FILES += $(MAIN_FILE)
# flatland/Primitive.cpp

include $(UTILS)/build.mk
