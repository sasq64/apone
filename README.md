utils
=====

This project contains a set of modules that can be included into your C++ project.

Each module is a single directory that contains the necessary cpp and h files for some functionality.
Each module can depend on other modules, and sometimes external libraries.

The modules use C++11 features so you need a recent compiler version.

To use the module system, you normally add the top directory to your include path, and then all cpp files inside
the module directories you want to use, for instance;

	MODULE_DIR := /home/myname/utils
	MODULES := coreutils archive ziplib

	CFLAGS := -I$(MODULE_DIR) -std=c++0x
	OBJFILES := $(patsubst %.cpp,%.o, $(wildcard $(addsuffix /*.cpp, $(MODULES))))

There is also a Makefile build system you can use by including the provided *Makefile.inc*. Check the top of that
file for its documentation.
