
CFLAGS :=
CXXFLAGS :=
OBJS :=
MODULES :=
LIBS :=
LDFLAGS :=
TARGET :=
FILES :=
INCLUDES :=
OBJDIR := obj/

SRC_PATTERNS := .cpp .cxx .cc .c .s .glsl

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

  APP_PLATFORM=android-10
  NDK_PLATFORM=android-14
  PREFIX=arm-linux-androideabi-
  TARGET_PRE=lib
  TARGET_EXT=.so
  CFLAGS += -DANDROID -DGL_ES
  LDFLAGS += -fPIC -Wl,-shared -no-canonical-prefixes -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now 

  # Test for prerequisites

  # First, we need the SDK. Find it from the path to the 'android' binary
  ifeq ($(ANDROID_SDK),)
    ANDROID_TOOL := $(realpath $(shell which android))
    ifneq ($(ANDROID_TOOL),)
      ANDROID_SDK := $(subst /tools/,,$(dir $(ANDROID_TOOL)))
      $(info Android SDK at $(ANDROID_SDK))
    else
      $(error Can not find Android SDK. Make sure the 'android' command is in your path.)
    endif
  else
    ANDROID_TOOL := $(ANDROID_SDK)/tools/android
  endif

  # Next, try to find the NDK relative to the SDK
  ifeq ($(ANDROID_NDK),)
    SDK_PARENT := $(dir $(ANDROID_SDK))
    ANDROID_NDK := $(lastword $(sort $(wildcard $(SDK_PARENT)*ndk*/)))
    ifneq ($(ANDROID_NDK),)
      $(info Found NDK at $(ANDROID_NDK))
    else
      $(error Can not find Android NDK)
    endif
  endif

  # Create a separate toolchain from the NDK if not created earlier
  ANDROID_TOOLCHAIN=$(HOME)/.cache/$(NDK_PLATFORM)-toolchain
  ifeq ($(realpath $(ANDROID_TOOLCHAIN)),)
    $(info Creating standalone toolchain for android)
    RES := $(shell $(ANDROID_NDK)/build/tools/make-standalone-toolchain.sh --toolchain=arm-linux-androideabi-clang3.3 --platform=$(NDK_PLATFORM) --install-dir=$(ANDROID_TOOLCHAIN))
    $(info $(RES))
  else
    $(info Android toolchain already created in $(ANDROID_TOOLCHAIN))
  endif
  
  ifeq ($(realpath $(ANDROID_TOOLCHAIN)/bin/$(PREFIX)as),)
    $(error Android toolchain is not valid)
  endif
  
  export PATH := $(PATH):$(ANDROID_TOOLCHAIN)/bin

  # We also need ant to build the final project
  ifeq ($(shell which ant),)
    $(error You need ant to build android projects)
  endif

else ifeq ($(HOST),emscripten)
	CC = emcc
	CXX = em++
 	TARGET_EXT = .html
 	CFLAGS += -DGL_ES
 	COMP_CFLAGS += -Wno-warn-absolute-paths
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

#ifneq ($(USE_CCACHE),)
#CC := ccache $(CC)
#CXX := ccache $(CXX)
#endif

LD := $(CXX)
ANT := ant

ifeq ($(AR),)
AR := ar
endif
RANLIB=ranlib
AS := $(PREFIX)as
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump
