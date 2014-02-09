ifeq ($(SQLITE3_INCLUDED),)
SQLITE3_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
LIBS += -ldl
INCLUDES += $(THIS_DIR)..
FILES += $(THIS_DIR)sqlite3.c

FILES := $(realpath $(FILES))

endif