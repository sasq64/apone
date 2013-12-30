ifeq ($(FREETYPE_INCLUDED),)
FREETYPE_INCLUDED = 1
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

ifeq ($(HOST),android)
  INCLUDES += $(THIS_DIR)android/include $(THIS_DIR)android/include/freetype2
  LDFLAGS += -L$(THIS_DIR)android/lib
  LIBS += -lpng -lfreetype
else ifeq ($(HOST),emscripten)
  INCLUDES += $(THIS_DIR)emscripten/include $(THIS_DIR)emscripten/include/freetype
  LDFLAGS += -L$(THIS_DIR)emscripten/lib
  LIBS += -lfreetype
else
  CFLAGS += `freetype-config --cflags` `libpng-config --cflags`
  LIBS += `freetype-config --libs` `libpng-config --libs`
endif

endif