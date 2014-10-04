ifeq ($(BACKWARDS_CPP_INCLUDED),)
BACKWARDS_CPP_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
LIBS += -lbfd -ldl
INCLUDES += $(THIS_DIR)..
CFLAGS += -DBACKWARD_CPP
FILES += $(THIS_DIR)backward.cpp

endif