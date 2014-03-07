CPP_MODS=../cpp-mods
ifeq ($(realpath $(CPP_MODS)/config.mk),)
  CPP_MODS=../../cpp-mods
endif

include $(CPP_MODS)/config.mk

GRAPPIX=../grappix
ifeq ($(realpath $(GRAPPIX)/module.mk),)
  GRAPPIX=..
endif

include $(GRAPPIX)/module.mk
include $(CPP_MODS)/coreutils/module.mk

# Uncomment to use clang instead
#CXX=clang++
#CC=clang

TARGET := test
LOCAL_FILES := test.cpp

include $(CPP_MODS)/build.mk
