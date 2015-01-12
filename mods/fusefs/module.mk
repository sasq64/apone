ifeq ($(FUSEFS_INCLUDED),)
FUSEFS_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

LIBS += `pkg-config fuse --libs`
CFLAGS += `pkg-config fuse --cflags`
INCLUDES += $(THIS_DIR)..

fusefs_FILES := $(THIS_DIR)fusefs.cpp

MODULES += fusefs

endif