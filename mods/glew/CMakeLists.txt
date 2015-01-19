cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.glew)

set(GLEW_DIR glew-1.11.0)

set(SOURCE_FILES
	${GLEW_DIR}/src/glew.c
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-common -fPIC")
include_directories(${GLEW_DIR}/include)
add_definitions(-DGLEW_NO_GLU -DGLEW_STATIC)

add_library(glew ${SOURCE_FILES})
