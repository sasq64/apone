ifeq ($(IMAGE_INCLUDED),)
IMAGE_INCLUDED = 1
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

INCLUDES += $(THIS_DIR)..
image_FILES := $(THIS_DIR)image.cpp $(THIS_DIR)packer.cpp

ifeq ($(HOST),android)
  image_INCLUDES := $(THIS_DIR)android/include
  LDFLAGS += -L$(THIS_DIR)android/lib
  LIBS += -lpng
else ifeq ($(HOST),emscripten)
else ifeq ($(HOST),raspberrypi)
  CFLAGS += -I/usr/include/libpng12
  LIBS += -lpng12
else
  image_CFLAGS := `libpng-config --cflags`
  LIBS += `libpng-config --libs`
endif

include $(THIS_DIR)../coreutils/module.mk

MODULES += image

endif