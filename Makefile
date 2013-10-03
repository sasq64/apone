
UTILS=../utils

OBJDIR=obj/
CFLAGS := -g -Wall -I. -I$(UTILS)
CXXFLAGS=-std=c++0x
TARGET=test

LIBS := -lglfw -lGL -lGLEW
OBJS := main.o shader.o
MODULES := $(UTILS)/coreutils

LINUX_CC=ccache clang -Qunused-arguments
LINUX_CXX=ccache clang++ -Qunused-arguments

include $(UTILS)/Makefile.inc