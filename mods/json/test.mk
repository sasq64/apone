include ../config.mk


include module.mk
include ../coreutils/module.mk

TARGET := jsontest
CFLAGS :=  -I../test -DUNIT_TEST
LOCAL_FILES := ../test/catch.cpp json.cpp

CC := clang
CXX := clang++

include ../build.mk
