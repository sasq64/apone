ifeq ($(FUSE_GRAPPIX_INCLUDED),)
FUSE_GRAPPIX_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

ifeq ($(CPP_MODS),)
  CPP_MODS := $(realpath $(THIS_DIR)/..)
endif
#CPP_MODS := $(realpath $(CPP_MODS))


INCLUDES += $(THIS_DIR)..
fusegrappix_FILES := $(THIS_DIR)fuse-grappix.cpp


MODULES += fusegrappix

include $(CPP_MODS)/fusefs/module.mk

endif