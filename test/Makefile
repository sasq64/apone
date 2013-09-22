OBJDIR := obj/
CFLAGS := -g -Wall
CXXFLAGS := -std=c++0x

UTILS = ..
CFLAGS += -I$(UTILS) -I../netlink
MODULES := $(UTILS)/coreutils $(UTILS)/bbsutils $(UTILS)/netlink

LDFLAGS := -pthread

TARGET := shelltest
OBJS := shelltest.o

CC := gcc-4.7
CXX := g++-4.7

include ../Makefile.inc
