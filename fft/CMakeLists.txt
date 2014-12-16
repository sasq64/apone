cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.fft)

find_library(FFT_LIBRARY fftw3f)
#find_path(FFT_PATH mpg123.h)
message("FFT " ${FFT_LIBRARY})
set(SOURCE_FILES spectrum.cpp)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++0x")
add_definitions(-DFFTW_SINGLE)

add_library(fft ${SOURCE_FILES})
target_link_libraries(fft fftw3f)
