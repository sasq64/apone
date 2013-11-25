C++ utils
=========

This project contains a set of modules that can be included into your C++ project.

Each module is a single directory that contains the necessary cpp and h files for some functionality.
Each module can depend on other modules, and sometimes external libraries.

The modules use C++11 features so you need a recent compiler version.

To use the module system, you normally add the top directory to your include path, and then all cpp files inside
the module directories you want to use. If you are using Makefiles, the easiest way is to use the provided
*Makefile.inc*, which simplifies the building process. This is a minimal Makefile example;

	MODULE_DIR := ../utils
	CFLAGS := -std=c++0x -I$(MODULE_DIR)
	MODULES := $(MODULE_DIR)/coreutils

	TARGET := test
	OBJS := test.o

	include $(MODULE_DIR)/Makefile.inc

If you do not wish to use *Makefile.inc* you need to either copy the modules into your source directory, or
set up appropriate build rules that can build from the modules source files into your own object directory.

Makefile.inc new proposal
=========================

include setup.mk

Investigates environment and set up default settings

Input variable, defaults to current system (linux or windows). Set to cross compile.
HOST = android | emscripten | raspberrypi | linux | windows

CC = correct compiler for host
CXX =
CFLAGS = Defines for host (-DRASPERRYPI etc)
OBJDIR = obj/$HOST

