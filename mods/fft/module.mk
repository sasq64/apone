ifeq ($(FFT_INCLUDED),)
FFT_INCLUDED = 1

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

INCLUDES += $(THIS_DIR)..
fft_FILES := $(THIS_DIR)spectrum.cpp

COMPILE_FFTW=1

ifneq ($(COMPILE_FFTW),)

FFTW := $(THIS_DIR)fftw-3.3.4


fft_CFLAGS := -DFFTW_SINGLE -DHAVE_CONFIG_H -I$(FFTW) -I$(FFTW)/kernel -I$(FFTW)/rdft -I$(FFTW)/dft -I$(FFTW)/reodft -O3  -fstrict-aliasing -fno-schedule-insns -ffast-math -fomit-frame-pointer
# -mtune=native -malign-double

fft_DIRS := $(FFTW)/kernel $(FFTW)/dft $(FFTW)/rdft $(FFTW)/reodft $(FFTW)/api


SIMD := scalar
#SIMD := simd/sse2
#fft_CFLAGS += -DHAVE_SSE2
#ifdef ARM
#SIMD := simd/neon
#fft_CFLAGS += -DHAVE_NEON -mfpu=neon
#endif

fft_DIRS += $(FFTW)/rdft/scalar $(FFTW)/rdft/scalar/r2cb $(FFTW)/rdft/scalar/r2cf $(FFTW)/rdft/scalar/r2r
fft_DIRS += $(FFTW)/dft/scalar $(FFTW)/dft/scalar/codelets
fft_CFLAGS += -I$(FFTW)/rdft/scalar -I$(FFTW)/dft/scalar

#fft_DIRS += $(FFTW)/rdft/$(SIMD) $(FFTW)/rdft/$(SIMD)/r2cb $(FFTW)/rdft/$(SIMD)/r2cf $(FFTW)/rdft/$(SIMD)/r2r
#fft_DIRS += $(FFTW)/dft/$(SIMD) $(FFTW)/dft/$(SIMD)/codelets
#fft_CFLAGS += -I$(FFTW)/simd-support -I$(FFTW)/rdft/simd -I$(FFTW)/dft/simd -I$(FFTW)/rdft/$(SIMD) -I$(FFTW)/dft/$(SIMD)
#fft_DIRS += $(FFTW)/simd-support

#endif

else

LIBS += -lfftw3

endif

MODULES += fft

endif