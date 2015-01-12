ifeq ($(BBSUTILS_INCLUDED),)
BBSUTILS_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
INCLUDES += $(THIS_DIR)..
bbsutils_DIRS := $(THIS_DIR)
LIBS += -pthread

MODULES += bbsutils

include $(THIS_DIR)../netlink/module.mk

endif