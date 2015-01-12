ifeq ($(TWEEN_INCLUDED),)
TWEEN_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

INCLUDES += $(THIS_DIR)..
tween_FILES := $(THIS_DIR)tween.cpp

include $(THIS_DIR)../coreutils/module.mk

MODULES += tween

endif