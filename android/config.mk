ifeq ($(ANDROID_INCLUDED),)
ANDROID_INCLUDED = 1
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

ANDROID_SDK_VERSION := 19
ANDROID_PACKAGE := mydomain.mypackage.myname
NDK_PLATFORM := android-19
PREFIX := arm-linux-androideabi-
TARGET_PRE := lib
TARGET_EXT := .apk
CFLAGS += -DANDROID -DGL_ES
LDFLAGS += -fPIC -Wl,-shared -no-canonical-prefixes -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now 
FIX_MANIFEST := $(THIS_DIR)fixManifest.py

endif