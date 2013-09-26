OBJDIR := obj/
UTILS = ..

CFLAGS := -g -Wall -DUNIT_TEST -I$(UTILS) -I../netlink -I.
CXXFLAGS := -std=c++0x 

MODULES := $(UTILS)/coreutils $(UTILS)/bbsutils $(UTILS)/netlink

LDFLAGS := -pthread

TARGET := test
OBJS := catch.o

CC := gcc-4.7
CXX := g++-4.7

include ../Makefile.inc
