
UTILS=../utils

OBJDIR=obj/
CFLAGS := -g -Wall -O0 -I. -I$(UTILS)
CXXFLAGS=-std=c++0x
TARGET=test

LIBS := -lglfw -lGL -lGLEW -lglut -lGLU -lm
OBJS := snake.o shader.o basic_buffer.o renderbuffer.o window.o image.o
MODULES := $(UTILS)/coreutils freetype-gl

CFLAGS += `freetype-config --cflags` -I/usr/X11/include -Ifreetype-gl
LIBS += `freetype-config --libs`


CFLAGS += `libpng-config --cflags`
LIBS += `libpng-config --libs`

LINUX_CC=ccache clang -Qunused-arguments
LINUX_CXX=ccache clang++ -Qunused-arguments

include $(UTILS)/Makefile.inc