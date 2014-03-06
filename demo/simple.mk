UTILS=../utils
GRAPPIX=grappix
CXX=clang++
CC=clang

include $(UTILS)/config.mk
include $(GRAPPIX)/module.mk

ifeq ($(HOST),android)
  ADK=/opt/arm-linux-androideabi
  SDK=/opt/android-sdk-linux
  APP_PLATFORM=android-10
  LDFLAGS += --sysroot=/opt/android-ndk-r9/platforms/android-14/arch-arm
  LOCAL_FILES += android/android_native_app_glue.c
  CFLAGS += -Iandroid 
  LDFLAGS += -lz -llog -landroid -lEGL -lGLESv2
else ifeq ($(HOST),emscripten)
endif

TARGET := simple
CFLAGS += -I. -Wall -O2
CXXFLAGS += -std=c++0x
DATA_FILES += data/ObelixPro.ttf data/tiles.png

LOCAL_FILES += simple.cpp

include $(UTILS)/build.mk
