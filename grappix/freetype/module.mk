ifeq ($(FREETYPE_INCLUDED),)
FREETYPE_INCLUDED = 1
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

ifeq ($(HOST),android)
  INCLUDES += $(THIS_DIR)android/include/freetype2
  LDFLAGS += -L$(THIS_DIR)android/lib
  LIBS += -lfreetype
else ifeq ($(HOST),emscripten)
  INCLUDES += $(THIS_DIR)emscripten/include $(THIS_DIR)emscripten/include/freetype
  LDFLAGS += -L$(THIS_DIR)emscripten/lib
  LIBS += -lfreetype
else
  CFLAGS += `freetype-config --cflags`
  LIBS += `freetype-config --libs`
endif

#CXXFLAGS += -Wno-literal-suffix

endif