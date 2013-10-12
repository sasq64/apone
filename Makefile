
UTILS=../utils

OBJDIR=obj/
CFLAGS := -g -Wall -O2 -I. -I$(UTILS)
CXXFLAGS=-std=c++0x
TARGET=snake

LIBS := -lglfw -lGL -lGLEW
OBJS := snake.o shader.o basic_buffer.o renderbuffer.o window.o image.o distancefield.o
OBJS += freetype-gl/texture-atlas.o freetype-gl/texture-font.o freetype-gl/vector.o freetype-gl/edtaa3func.o
MODULES := $(UTILS)/coreutils

CFLAGS += `freetype-config --cflags` -I/usr/X11/include -Ifreetype-gl
LIBS += `freetype-config --libs`

CFLAGS += `libpng-config --cflags`
LIBS += `libpng-config --libs`

LINUX_CC=ccache clang -Qunused-arguments
LINUX_CXX=ccache clang++ -Qunused-arguments

include $(UTILS)/Makefile.inc