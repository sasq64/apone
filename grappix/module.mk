ifeq ($(GRAPPIX_INCLUDED),)
GRAPPIX_INCLUDED = 1
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

INCLUDES += $(THIS_DIR).. $(THIS_DIR)freetype-gl
DIRS += $(THIS_DIR) $(THIS_DIR)freetype-gl $(THIS_DIR)shaders
CFLAGS +=  -DWITH_FREETYPE

ifeq ($(HOST),android)
  DIRS += $(THIS_DIR)android
  LIBS += -lz -llog -landroid -lEGL -lGLESv2
else ifeq ($(HOST),emscripten)
  DIRS += $(THIS_DIR)pc
else ifeq ($(HOST),apple)
  DIRS += $(THIS_DIR)pc
  LIBS += -lglfw -framework OpenGL -lGLEW
else
  DIRS += $(THIS_DIR)pc
  LIBS += -lglfw -lGL -lGLEW
endif

include $(THIS_DIR)../freetype/module.mk
include $(UTILS)/coreutils/module.mk

endif