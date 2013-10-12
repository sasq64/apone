
UTILS=../utils

OBJDIR=obj/
CFLAGS := -g -Wall -O0 -I. -I$(UTILS) -Ifreetype-gl
CXXFLAGS=-std=c++0x
TARGET=tiles

LINUX_LIBS := -lglfw -lGL -lGLEW
OBJS := ${TARGET}.o shader.o basic_buffer.o renderbuffer.o image.o distancefield.o
OBJS += freetype-gl/texture-atlas.o freetype-gl/texture-font.o freetype-gl/vector.o freetype-gl/edtaa3func.o
MODULES := $(UTILS)/coreutils

LINUX_OBJS := window.o
ANDROID_OBJS := window_android.o
ANDROID_CFLAGS := -I/home/sasq/android/include/freetype2 -I/home/sasq/android/include
ANDROID_LDFLAGS := -L/home/sasq/android/lib
ANDROID_LIBS := -lpng -lfreetype -landroid -lEGL -lGLESv2 

LINUX_CFLAGS += `freetype-config --cflags`
LINUX_LIBS += `freetype-config --libs`
LINUX_CFLAGS += `libpng-config --cflags`
LINUX_LIBS += `libpng-config --libs`

LINUX_CC=ccache clang -Qunused-arguments
LINUX_CXX=ccache clang++ -Qunused-arguments

include $(UTILS)/Makefile.inc