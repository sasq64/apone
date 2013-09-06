utils
=====

This project contains a set of modules that can be included into your C++ project.

Each module is a single directory that contains the necessary cpp and include files for some functionality.
Each module can depend on other modules, and sometimes external libraries.

To use the module system, you normally add the top directory to your include path, and then all cpp files inside
the module directories you want to use, for instance;

	CFLAGS += -I$(MODULE_DIR)
	MODULES := coreutils archive ziplib
	OBJFILES := $(patsubst %.cpp,%.o, $(wildcard $(addsuffix /*.cpp, $(MODULES))))
