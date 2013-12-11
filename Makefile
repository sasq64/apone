UTILS=../utils
CXX=clang++
CC=clang
USE_CCACHE=1

include $(UTILS)/config.mk

OBJDIR := obj/
TARGET := grappix
CFLAGS += -Wall -O2 -I. -I$(UTILS) -Ifreetype-gl -DWITH_FREETYPE
CXXFLAGS += -std=c++0x

CHIPM=../chipmachine

CFLAGS += -I$(CHIPM)/src -I$(CHIPM)/src/plugins/ModPlugin -I$(CHIPM)/src/plugins/VicePlugin

include $(UTILS)/coreutils/module.mk

ifeq ($(HOST),android)
  ADK=/opt/arm-linux-androideabi
  SDK=/opt/android-sdk-linux
  APP_PLATFORM=android-10

  LDFLAGS += --sysroot=/opt/android-ndk-r9/platforms/android-14/arch-arm
  LOCAL_FILES += android/window_android.cpp android/android_native_app_glue.c
  CFLAGS += -Iandroid -I$(ADK)/include -I$(ADK)/include/freetype2
  LIBS += $(ADK)/lib/libfreetype.a $(ADK)/lib/libpng.a -lz -llog -landroid -lEGL -lGLESv2
else ifeq ($(HOST),emscripten)
  CFLAGS += -Ifreetype/include
  #CFLAGS += -I$(EMSCRIPTEN)/system/include/freetype2 -s ASM_JS=1
  LDFLAGS += -Lfreetype --preload-file data
  # --preload-file fonts -s OUTLINING_LIMIT=50000
  LDFLAGS += -L$(CHIPM)/src/plugins/VicePlugin/em -L$(CHIPM)/src/plugins/ModPlugin
  # -s TOTAL_MEMORY=33554432
  # -s DISABLE_EXCEPTION_CATCHING=0
  LIBS += -lfreetype
  #-lSDL -lz -lglfw -lGL
  LOCAL_FILES += window.cpp
  TARGETDIR := html/
else
  CFLAGS += `freetype-config --cflags` `libpng-config --cflags`
  LIBS += `freetype-config --libs` `libpng-config --libs` -lSDL -lglfw -lGL -lGLEW
  LIBS += -lviceplugin -lmodplugin
  LOCAL_FILES += window.cpp
  LDFLAGS +=-L$(CHIPM)/src/plugins/ModPlugin -L$(CHIPM)/src/plugins/VicePlugin
endif

## Hack that lets us run the currently open file from Sublime if it is one of the main files
MAIN_FILES = main.cpp snake.cpp tiletest2.cpp bobs.cpp simple.cpp blur.cpp map.cpp
MAIN_FILE := bobs.cpp
ifneq ($(ACTIVEFILE),)
 ifneq ($(findstring $(ACTIVEFILE),$(MAIN_FILES)),)
  MAIN_FILE := $(ACTIVEFILE)
 endif
endif


LOCAL_FILES += $(MAIN_FILE) tiles.cpp shader.cpp font.cpp render_target.cpp texture.cpp tween.cpp image.cpp color.cpp
LOCAL_FILES += distancefield.cpp freetype-gl/texture-atlas.c freetype-gl/texture-font.c freetype-gl/vector.c freetype-gl/edtaa3func.c
LOCAL_FILES += $(wildcard shaders/*.glsl)



include $(UTILS)/build.mk
