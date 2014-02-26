ifeq ($(GRAPPIX_INCLUDED),)
GRAPPIX_INCLUDED = 1
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

INCLUDES += $(THIS_DIR).. $(THIS_DIR)freetype-gl
DIRS += $(THIS_DIR) $(THIS_DIR)freetype-gl $(THIS_DIR)shaders
CFLAGS +=  -DWITH_FREETYPE

ifeq ($(HOST),android)
  DIRS += $(THIS_DIR)android
  LDFLAGS += -lz -llog -landroid -lEGL -lGLESv2
else ifeq ($(HOST),emscripten)
  DIRS += $(THIS_DIR)pc
  #INCLUDES += $(THIS_DIR)freetype/include
  #LDFLAGS += -L$(THIS_DIR)freetype
  #LIBS += -lfreetype
else
  DIRS += $(THIS_DIR)pc
  #CFLAGS += `freetype-config --cflags` `libpng-config --cflags`
  #LIBS += `freetype-config --libs` `libpng-config --libs`
  LIBS += -lSDL -lglfw -lGL -lGLEW
endif

include $(THIS_DIR)../freetype/module.mk
include $(UTILS)/coreutils/module.mk

endif