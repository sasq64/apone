
CFLAGS :=
CXXFLAGS :=
OBJS :=
MODULES :=
LIBS :=

ifeq ($(CC),cc)
CC = gcc
endif

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
	PREFIX=arm-linux-androideabi-
	TARGET_PRE=lib
	TARGET_EXT=.so
 	CFLAGS := $(CFLAGS) -DANDROID
else ifeq ($(HOST),emscripten)
	CC = emcc
	CXX = em++
 	TARGET_EXT = .html
 	CFLAGS += -DGL_ES
else ifeq ($(HOST),raspberrypi)
	CFLAGS += -DRASPBERRYPI
endif

ifneq ($(ARM),)
	CFLAGS += -DARM
endif

ifndef CGC
CGC := cgc
endif
ifndef XXD
XXD := xxd
endif


CC := $(PREFIX)$(CC)$(C_VERSION)
CXX := $(PREFIX)$(CXX)$(C_VERSION)

LD := $(CXX)

AR := ar
AS := $(PREFIX)as
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump
