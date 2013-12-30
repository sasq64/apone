ifeq ($(WEBUTILS_INCLUDED),)
WEBUTILS_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

INCLUDES += $(THIS_DIR)..
ifeq ($(HOST),emscripten)
FILES += $(THIS_DIR)webgetter_em.cpp
else
FILES += $(THIS_DIR)webgetter.cpp $(THIS_DIR)webrpc.cpp 
LIBS += -pthread -lcurl
endif

FILES := $(realpath $(FILES))

endif