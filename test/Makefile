OBJDIR := obj/
CFLAGS := -g -Wall
CXXFLAGS := -std=c++0x

UTILS = ..
CFLAGS += -I$(UTILS) -I../netlink
MODULES := $(UTILS)/coreutils $(UTILS)/bbsutils $(UTILS)/webutils $(UTILS)/netlink

TARGET := utest
LIBS := -lz
OBJS := bbstest.o

LINUX_CC := gcc-4.7
LINUX_CXX := g++-4.7

LINUX_CFLAGS := $(LINUX_CFLAGS) `curl-config --cflags`
LINUX_LIBS := `curl-config --libs`

all : start_rule

run :
	./utest

include Makefile.inc
