ifeq ($(BBSUTILS_INCLUDED),)
BBSUTILS_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
INCLUDES += $(THIS_DIR)..
MODULES += $(THIS_DIR)
LIBS += -pthread -lz

include $(THIS_DIR)../netlink/module.mk

endif