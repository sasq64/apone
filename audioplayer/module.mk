ifeq ($(AUDIO_PLAYER_INCLUDED),)
AUDIO_PLAYER_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
LIBS += -ldl
INCLUDES += $(THIS_DIR)..
FILES += $(THIS_DIR)audioplayer.cpp
FILES := $(realpath $(FILES))

ifeq ($(HOST),linux)
LIBS += -lasound
endif

endif