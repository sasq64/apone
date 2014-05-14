ifeq ($(FFT_INCLUDED),)
FFT_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

INCLUDES += $(THIS_DIR)..
fft_FILES := $(THIS_DIR)spectrum.cpp

LIBS += -lrfftw -lfftw

MODULES += fft

endif