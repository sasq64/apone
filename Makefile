
UTILS=../utils

OBJDIR=obj/
CFLAGS := -g -Wall -I. -I$(UTILS)
CXXFLAGS=-std=c++0x
TARGET=test

LIBS := -lglfw -lGL -lGLEW
OBJS := main.o shader.o basic_buffer.o renderbuffer.o window.o image.o
MODULES := $(UTILS)/coreutils

CFLAGS += `libpng-config --cflags`
LIBS += `libpng-config --libs`

LINUX_CC=ccache clang -Qunused-arguments
LINUX_CXX=ccache clang++ -Qunused-arguments

include $(UTILS)/Makefile.inc