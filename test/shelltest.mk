OBJDIR := obj/
UTILS = ..

CFLAGS := -g -Wall -I$(UTILS) -I../netlink
CXXFLAGS := -std=c++0x 

MODULES := $(UTILS)/coreutils $(UTILS)/bbsutils $(UTILS)/netlink

LDFLAGS := -pthread

TARGET := shelltest
OBJS := shelltest.o

CC := gcc-4.7
CXX := g++-4.7

include ../Makefile.inc
