C++ utils
=========

This project contains a set of modules that can be included into your C++ project.

Each module is a single directory that contains the necessary cpp and h files for some functionality.
Each module can depend on other modules, and sometimes external libraries.

The modules use C++11 features so you need a recent compiler version.

The module system is based on make. This is a minimal for a project makefile that uses
this system;

	# First point to the utils directory
	MODULE_DIR = ../utils
	# then we include config.mk to set up variables and detect host etc
	include $(MODULE_DIR)/config.mk

	# then we define our compilation target and the sources to include
	TARGET := test
	LOCAL_FILES += test.cpp
	# and finally we build everything by including the build rules
	include $(MODULE_DIR)/build.mk

A slightly more advanced example. 

	MODULE_DIR = ../utils
	include $(MODULE_DIR)/config.mk

	# Include the code modules that we want to use
	include $(MODULE_DIR)/coreutils/module.mk
	include $(MODULE_DIR)/sqlite3/module.mk

	TARGET := dbtest
	LOCAL_FILES += dbtest.cpp
	# Include all source files in the dbsrc/ directory
	LOCAL_MODULES += dbsrc

	# Include system specific sources depending on host system
	ifeq ($(HOST),android)
	LOCAL_FILES += android_sys.c
	else
	LOCAL_FILES += pc_sys.c
	endif

	include $(MODULE_DIR)/build.mk
