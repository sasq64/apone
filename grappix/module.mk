ifeq ($(GRAPPIX_INCLUDED),)
GRAPPIX_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
MODULES += $(THIS_DIR) $(THIS_DIR)/pc $(THIS_DIR)/freetype-gl $(THIS_DIR)/shaders

CFLAGS += `freetype-config --cflags` `libpng-config --cflags` -I$(THIS_DIR)/freetype-gl -DWITH_FREETYPE
LIBS += `freetype-config --libs` `libpng-config --libs` -lSDL -lglfw -lGL -lGLEW

include $(UTILS)/coreutils/module.mk

endif