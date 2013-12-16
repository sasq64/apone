UTILS=../utils
GRAPPIX=grappix
CXX=clang++
CC=clang

include $(UTILS)/config.mk

OBJDIR := obj/
TARGET := simple
CFLAGS += -Wall -O2 -I. -I$(UTILS)
CXXFLAGS += -std=c++0x

LDFLAGS += --preload-file data

include $(GRAPPIX)/module.mk

LOCAL_FILES += simple.cpp

include $(UTILS)/build.mk
