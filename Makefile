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

LIBS += -lviceplugin
CFLAGS += -I$(CHIPM)/src -I$(CHIPM)/src/plugins/ModPlugin -I$(CHIPM)/src/plugins/VicePlugin

ifeq ($(HOST),android)
  ADK=/opt/arm-linux-androideabi
  SDK=/opt/android-sdk-linux
  APP_PLATFORM=android-10

  LDFLAGS += --sysroot=/opt/android-ndk-r9/platforms/android-14/arch-arm
  OBJS += android/window_android.o android/android_native_app_glue.o
  CFLAGS += -Iandroid -I$(ADK)/include -I$(ADK)/include/freetype2
  LIBS += $(ADK)/lib/libfreetype.a $(ADK)/lib/libpng.a -lz -llog -landroid -lEGL -lGLESv2
else ifeq ($(HOST),emscripten)
  CFLAGS += -Ifreetype/include -s ASM_JS=1
  #CFLAGS += -I$(EMSCRIPTEN)/system/include/freetype2 -s ASM_JS=1
  LDFLAGS += -Lfreetype --preload-file data --preload-file fonts
  LDFLAGS += -L$(CHIPM)/src/plugins/VicePlugin -L$(CHIPM)/src/plugins/ModPlugin -s TOTAL_MEMORY=33554432
  LIBS += -lfreetype 
  #-lSDL -lz -lglfw -lGL
  OBJS += window.o
else
  CFLAGS += `freetype-config --cflags` `libpng-config --cflags`
  LIBS += `freetype-config --libs` `libpng-config --libs` -lSDL -lglfw -lGL -lGLEW
  OBJS += window.o
  LDFLAGS +=-L$(CHIPM)/src/plugins/ModPlugin -L$(CHIPM)/src/plugins/VicePlugin
endif

MAIN_FILES = main.cpp snake.cpp tiletest.cpp bobs.cpp simple.cpp blur.cpp map.cpp
MAINOBJ := main.o

OBJS += tiles.o shader.o basic_buffer.o texture.o tween.o image.o
OBJS += distancefield.o freetype-gl/texture-atlas.o freetype-gl/texture-font.o freetype-gl/vector.o freetype-gl/edtaa3func.o

MODULES += $(UTILS)/coreutils
SHADERS = $(patsubst %.glsl,%.o, $(wildcard shaders/*.glsl))
OBJS += $(SHADERS)

include $(UTILS)/build.mk
