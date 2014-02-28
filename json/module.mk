ifeq ($(JSON_INCLUDED),)
JSON_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

INCLUDES += $(THIS_DIR)..
json_FILES := $(THIS_DIR)json.cpp
json_DIRS := $(THIS_DIR)json-c
json_CFLAGS := -DHAVE_CONFIG_H -Wall-Wextra -Wwrite-strings -Wno-unused-parameter -std=gnu99 -D_GNU_SOURCE -D_REENTRANT -fPIC -DPIC
json_INCLUDES := $(THIS_DIR)json-c

MODULES += json

endif

#-g -O2 -MT json_tokener.lo -MD -MP -MF .deps/json_tokener.Tpo -c json_tokener.c  -fPIC -DPIC -o .libs/json_tokener.o
