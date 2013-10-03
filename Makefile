
UTILS=../utils

OBJDIR=obj/
CFLAGS := -DGLEW_NO_GLU -g -Wall -I. -I$(UTILS)
CXXFLAGS=-std=c++0x
TARGET=test

LIBS := -lglfw -lGL -lGLEW
OBJS := main.o shader.o
MODULES := $(UTILS)/coreutils

LINUX_CC=clang
LINUX_CXX=clang++
LINUX_MODULES := host/linux
LINUX_CFLAGS := -DLINUX
LINUX_LIBS := -lGL -lglfw


include $(UTILS)/Makefile.inc