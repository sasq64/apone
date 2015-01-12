include ../config.mk

include module.mk
include ../coreutils/module.mk

CFLAGS += -I../test -DIMAGE_UNIT_TEST

OBJDIR := obj/

TARGET := imagetest
FILES += $(image_FILES) ../test/catch.cpp
image_FILES :=

include ../build.mk
