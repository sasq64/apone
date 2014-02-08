ifeq ($(CRYPTO_INCLUDED),)
CRYPTO_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

INCLUDES += $(THIS_DIR)..
FILES += $(THIS_DIR)sha256.cpp

FILES := $(realpath $(FILES))

endif