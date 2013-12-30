UTILS=../utils
GRAPPIX=grappix
CXX=clang++
CC=clang

include $(UTILS)/config.mk
include $(GRAPPIX)/module.mk

TARGET := simple
CFLAGS += -I. -Wall -O2
CXXFLAGS += -std=c++0x
DATA_FILES += data/ObelixPro.ttf data/tiles.png

LOCAL_FILES += simple.cpp

include $(UTILS)/build.mk
