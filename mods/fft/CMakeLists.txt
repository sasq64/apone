cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.fft)

find_library(FFT_LIBRARY fftw3f)
find_path(FFT_PATH fftw3.h)
#message("FFT " ${FFT_LIBRARY} "/" ${FFT_PATH})
#find_package()

#set(FFTW fftw-3.3.4)

#set(FFT_DIRS
#	${FFTW}/kernel ${FFTW}/dft ${FFTW}/rdft ${FFTW}/reodft ${FFTW}/api
#	${FFTW}/rdft/scalar ${FFTW}/rdft/scalar/r2cb ${FFTW}/rdft/scalar/r2cf ${FFTW}/rdft/scalar/r2r
#	${FFTW}/dft/scalar ${FFTW}/dft/scalar/codelets
#)

#SIMD := scalar
#SIMD := simd/sse2
#fft_CFLAGS += -DHAVE_SSE2
#ifdef ARM
#SIMD := simd/neon
#fft_CFLAGS += -DHAVE_NEON -mfpu=neon
#endif

set(SOURCE_FILES spectrum.cpp)

#foreach(DIR ${FFT_DIRS})
#	message(${DIR})
#	file(GLOB COMP_FILES ${DIR}/*.c)
#	set(SOURCE_FILES ${SOURCE_FILES} ${COMP_FILES})
#endforeach()

#add_definitions(-DFFTW_SINGLE -DHAVE_CONFIG_H)
#include_directories(${FFTW} ${FFTW}/api ${FFTW}/kernel ${FFTW}/rdft ${FFTW}/dft ${FFTW}/reodft ${FFTW}/rdft/scalar ${FFTW}/dft/scalar)
# -O3  -fstrict-aliasing -fno-schedule-insns -ffast-math -fomit-frame-pointer



add_definitions(-DFFTW_SINGLE)
include_directories(${FFT_PATH})

add_library(fft ${SOURCE_FILES})
target_link_libraries(fft ${FFT_LIBRARY})
