ifeq ($(AUDIO_PLAYER_INCLUDED),)
AUDIO_PLAYER_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
LIBS += -ldl

INCLUDES += $(THIS_DIR)..
audioplayer_FILES := $(THIS_DIR)audioplayer.cpp

ifeq ($(HOST),linux)
LIBS += -lasound
endif

MODULES += audioplayer

endif