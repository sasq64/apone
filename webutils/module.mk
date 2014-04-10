ifeq ($(WEBUTILS_INCLUDED),)
WEBUTILS_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

INCLUDES += $(THIS_DIR)..

ifeq ($(HOST),emscripten)
  webutils_FILES := $(THIS_DIR)webgetter_em.cpp
else ifeq ($(HOST),android)
  webutils_INCLUDES := $(THIS_DIR)android/include
  webutils_FILES := $(THIS_DIR)webgetter.cpp $(THIS_DIR)webrpc.cpp 
  LDFLAGS += -L$(THIS_DIR)android/lib
  LIBS += -lcurl
else
  webutils_FILES := $(THIS_DIR)webgetter.cpp $(THIS_DIR)webrpc.cpp 
  LIBS += -pthread -lcurl
endif

MODULES += webutils

endif