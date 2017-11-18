cmake_minimum_required(VERSION 3.5)
project(cpp-mods.fft)

find_library(FFT_LIBRARY NAMES libfftw3f.a fftw3f)
find_path(FFT_PATH fftw3.h)

set(SOURCE_FILES spectrum.cpp)

add_library(fft ${SOURCE_FILES})
target_compile_definitions(fft PRIVATE -DFFTW_SINGLE)
target_include_directories(fft PRIVATE ${FFT_PATH})
target_link_libraries(fft PRIVATE ${FFT_LIBRARY})
