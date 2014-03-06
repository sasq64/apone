#

ifneq (,$(findstring clang,$(CC)))
  CC := $(subst $(ANDROID_PREFIX),,$(CC))
endif

ifneq (,$(findstring clang,$(CXX)))
  CXX := $(subst $(ANDROID_PREFIX),,$(CXX))
endif

CXX := $(subst ccache,,$(CXX))
CC := $(subst ccache,,$(CC))
LD := $(subst ccache,,$(LD))

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
RES := $(shell $(ANDROID_NDK)/build/tools/make-standalone-toolchain.sh --toolchain=arm-linux-androideabi-4.8 --llvm-version=3.3 --platform=$(NDK_PLATFORM) --install-dir=$(ANDROID_TOOLCHAIN))
$(info $(RES))
else
$(info Android toolchain already created in $(ANDROID_TOOLCHAIN))
endif

ifeq ($(realpath $(ANDROID_TOOLCHAIN)/bin),)
$(error Android toolchain is not valid)
endif

export PATH := $(ANDROID_TOOLCHAIN)/bin:$(PATH)

# We also need ant to build the final project
ifeq ($(shell which ant),)
$(error You need ant to build android projects)
endif

ifeq ($(ANDROID_PROJECT),)
$(error You need to set ANDROID_PROJECT to point to your android project directory)
endif

ifeq ($(APP_NAME),)
APP_NAME := $(TARGET)
endif

LDFLAGS += --sysroot=$(ANDROID_NDK)/platforms/$(NDK_PLATFORM)/arch-arm
FILES += $(ANDROID_NDK)/sources/android/native_app_glue/android_native_app_glue.c
CFLAGS += -I$(ANDROID_NDK)/sources/android
