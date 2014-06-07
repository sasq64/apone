ifeq ($(FFT_INCLUDED),)
FFT_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

INCLUDES += $(THIS_DIR)..
fft_FILES := $(THIS_DIR)spectrum.cpp
#LIBS += -lfftw3

FFTW := $(THIS_DIR)fftw-3.3.4


fft_CFLAGS := -DHAVE_CONFIG_H -I$(FFTW) -I$(FFTW)/kernel -I$(FFTW)/rdft -I$(FFTW)/rdft/scalar -I$(FFTW)/dft -I$(FFTW)/dft/scalar -I$(FFTW)/reodft -O3 -fomit-frame-pointer -fstrict-aliasing -fno-schedule-insns -ffast-math
# -mtune=native -malign-double
fft_DIRS := $(FFTW)/kernel $(FFTW)/dft $(FFTW)/rdft $(FFTW)/reodft $(FFTW)/api

ifdef ARM
else
fft_DIRS += $(FFTW)/rdft/scalar $(FFTW)/rdft/scalar/r2cb $(FFTW)/rdft/scalar/r2cf $(FFTW)/rdft/scalar/r2r
fft_DIRS += $(FFTW)/dft/scalar $(FFTW)/dft/scalar/codelets
endif


#$(FFTW)/simd-support

MODULES += fft

endif