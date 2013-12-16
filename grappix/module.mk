ifeq ($(GRAPPIX_INCLUDED),)
GRAPPIX_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
MODULES += $(THIS_DIR) $(THIS_DIR)/freetype-gl $(THIS_DIR)/shaders
CFLAGS += -I$(THIS_DIR)/freetype-gl -DWITH_FREETYPE

ifeq ($(HOST),android)
  MODULES += $(THIS_DIR)/android
else ifeq ($(HOST),emscripten)
  MODULES += $(THIS_DIR)/pc
  CFLAGS += -I$(THIS_DIR)/freetype/include
  LDFLAGS += -L$(THIS_DIR)/freetype
  LIBS += -lfreetype
else
  MODULES += $(THIS_DIR)/pc
  CFLAGS += `freetype-config --cflags` `libpng-config --cflags`
  LIBS += `freetype-config --libs` `libpng-config --libs` -lSDL -lglfw -lGL -lGLEW
endif

include $(UTILS)/coreutils/module.mk

endif