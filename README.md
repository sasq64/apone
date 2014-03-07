# C++ MODS

## Introduction

*C++ MODS* is two things;

* It's a set of code modules that you can use in your C++ projects
* It contains a set of make files which constitutes a 'build system' on top of *make*

To use the modules in this project, you are encouraged to include the makefiles from
C++ MODS. This gives you a lot of functionality; they both let you write smaller makefiles
and facilitates cross platform development.

Currently supported are *Linux*, *Mac OS*, *Android* and *emscripten*.

## The Build System

*C++ MODS* comes with its own build system and is based on make. The basic idea
is that you include 'smart' makefiles that helps with the build.

First you include `config.mk` that figures out what the target of the build is
and sets up a bunch of variables, most importantly `$(HOST)` that can be used
to turn on/off other features depending on your compilation target.

Then you include `module.mk` from each module directory you want to use.

Last you include `build.mk` which contains all the targets and logic for
building your project given the variables you have defined.

`build.mk` and `config.mk` are not dependent on each other; if you only want
help with configuration in your normal Makefile you can just include `config.mk`,
and if you don't care about cross compilation and want to set all flags yourself,
you can skip configuration and only include `build.mk`.

This is a small makefile for a project that uses this system;

	# First point to the cpp-mods directory
	MODULE_DIR = ../cpp-mods
	# then we include config.mk to set up variables and detect host etc
	include $(MODULE_DIR)/config.mk

	# then we define our compilation target and the sources to include
	TARGET := test
	LOCAL_FILES += test.cpp
	# and finally we build everything by including the build rules
	include $(MODULE_DIR)/build.mk

A slightly more advanced example;

	MODULE_DIR = ../cpp-mods
	include $(MODULE_DIR)/config.mk

	# Include the code modules that we want to use
	include $(MODULE_DIR)/coreutils/module.mk
	include $(MODULE_DIR)/sqlite3/module.mk

	CC := clang
	CXX := clang++

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

### Setting flags

#### Choosing the compiler

## Platform support

### Desktop (Linux, Mac)

Build using `make`
Requires gcc 4.7+ or clang 3.2+

### Android

Build using `HOST=android make`

Requires

* Android NDK r9+
* Android SDK api 19+
* ant

The *android* tool needs to be in your path and the NDK and SDK installed in the same parent directory.
Otherwise you need to set ANDROID_SDK and ANDROID_NDK to point to those directories.

### Emscripten

Build using `emmake make`

Requires: *emscripten*, and *emmake* in your path.

# Quickstart

```Shell
$ git clone http://github.com/sasq64/cpp-mods.git
$ git clone http://github.com/sasq64/grappix.git
$ cp -a grappix/quickstart .
$ cd quickstart
$ make
```
