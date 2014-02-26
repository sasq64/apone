OBJDIR := obj/
UTILS = ..

CFLAGS := -g -Wall -I$(UTILS) -I../netlink
CXXFLAGS := -std=c++0x 

DIRS := $(UTILS)/coreutils $(UTILS)/bbsutils $(UTILS)/netlink

LDFLAGS := -pthread

TARGET := simplebbs
OBJS := simplebbs.o

CC := gcc-4.7
CXX := g++-4.7

include ../Makefile.inc
