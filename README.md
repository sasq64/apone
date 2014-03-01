C++ utils
=========

This project contains a set of modules that can be included into your C++ project.

The modules use C++11 features so you need a recent compiler version.

The module system comes with its own build system is based on make. The basic idea
is that you include 'smart' makefiles that helps with the build.

First you include `config.mk` that figures out what the target of the build is
and sets up a bunch of variables, most importantly `$(HOST)` that can be used
to turn on/off other features depending on your compilation target.

Then you include `module.mk` from each module directory you want to use.

Last you include `build.mk` which contains all the targets and logic for
building your project given the variables you have defined.

`build.mk` and `config.mk` are not dependant on each other; if you only want
help with configuration in your normal Makefile you can just include `config.mk`,
and if you don't care about cross compilation and want to set all flags yourself,
you can skip configuration and only include `build.mk`.

This is a minimal for a project makefile that uses this system;

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
	LOCAL_DIRS += dbsrc

	# Include system specific sources depending on host system
	ifeq ($(HOST),android)
	LOCAL_FILES += android_sys.c
	else
	LOCAL_FILES += pc_sys.c
	endif

	include $(MODULE_DIR)/build.mk
