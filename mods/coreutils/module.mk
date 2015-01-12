ifeq ($(COREUTILS_INCLUDED),)
COREUTILS_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

INCLUDES += $(THIS_DIR)..
coreutils_DIRS := $(THIS_DIR)

MODULES += coreutils

endif