THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

CFLAGS := -I$(THIS_DIR)
CXXFLAGS := -std=c++0x
OBJS :=
DIRS :=
LIBS :=
LDFLAGS :=
TARGET :=
FILES :=
INCLUDES :=
MODULES :=
DIRS :=
PREFIX :=
C_VERSION :=
OBJDIR := obj/

SRC_PATTERNS := .cpp .cxx .cc .c .s .glsl

#ifeq ($(CC),cc)
#CC = gcc
#endif

# Figure out HOST if it is not set
ifeq ($(HOST),)

ifneq ($(EMSCRIPTEN),)
	HOST = emscripten
else ifeq ($(OS),Windows_NT)
	HOST = windows
	ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
    endif
else
    UNAME_S := $(shell uname -s)
    UNAME_N := $(shell uname -n)
    ifeq ($(UNAME_S),Linux)
    	HOST = linux
    endif
    ifeq ($(UNAME_N),raspberrypi)
    	HOST = raspberrypi
    	ARM = 1
    endif

    ifeq ($(UNAME_S),Darwin)
    	HOST = apple
    endif

    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)

    endif

    ifneq ($(filter %86,$(UNAME_P)),)
        #CCFLAGS += -D IA32
    endif

    ifneq ($(filter arm%,$(UNAME_P)),)
        ARM = 1
    endif
endif

endif

ifeq ($(HOST),android)

	include $(THIS_DIR)android/config.mk

else ifeq ($(HOST),emscripten)
	CC := emcc
	CXX := em++
 	TARGET_EXT := .html
 	CFLAGS += -DGL_ES
 	COMP_CFLAGS += -Wno-warn-absolute-paths
else ifeq ($(HOST),raspberrypi)

   PREFIX := arm-linux-gnueabihf-

  ifeq ($(PI_SDK),)
	PI_CC := $(realpath $(shell which $(PREFIX)gcc))
	ifneq ($(PI_CC),)
    PI_SDK := $(realpath $(dir $(PI_CC))/../../..)
	else
    $(error Can not find Rasberry PI compiler $(PREFIX)gcc in path)
	endif
  endif

  $(info Rasberry PI SDK at $(PI_SDK))
 
  CFLAGS += -DRASPBERRYPI -DLINUX -march=armv6 -mfpu=vfp -mfloat-abi=hard
  CFLAGS += --sysroot=$(PI_SDK)
  LDFLAGS += --sysroot=$(PI_SDK)

else ifeq ($(HOST),linux)
  CFLAGS += -DLINUX
endif


ifneq ($(ARM),)
	CFLAGS += -DARM
endif
