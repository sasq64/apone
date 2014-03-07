ifeq ($(GRAPPIX_INCLUDED),)
GRAPPIX_INCLUDED = 1
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

CPP_MODS := $(realpath $(CPP_MODS))
ifeq ($(CPP_MODS),)
  CPP_MODS := $(realpath ../cpp-mods)
endif

ifeq ($(CPP_MODS),)
  $(error Can not find 'cpp-mods'. Please set CPP_MODS to point to that directory)
endif

INCLUDES += $(THIS_DIR) $(THIS_DIR)grappix/freetype-gl
DIRS += $(THIS_DIR)grappix $(THIS_DIR)grappix/freetype-gl $(THIS_DIR)grappix/shaders
CFLAGS +=  -DWITH_FREETYPE

ifeq ($(HOST),android)
  DIRS += $(THIS_DIR)grappix/android
  LIBS += -lz -llog -landroid -lEGL -lGLESv2
else ifeq ($(HOST),emscripten)
  DIRS += $(THIS_DIR)grappix/pc
else ifeq ($(HOST),apple)
  DIRS += $(THIS_DIR)grappix/pc
  LIBS += -lglfw -framework OpenGL -lGLEW
else
  DIRS += $(THIS_DIR)grappix/pc
  LIBS += -lglfw -lGL -lGLEW
endif

include $(THIS_DIR)freetype/module.mk
include $(CPP_MODS)/coreutils/module.mk

endif