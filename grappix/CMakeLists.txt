cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.grappix)

file(GLOB MAIN_FILES grappix/*.cpp)
file(GLOB FREETYPE_FILES grappix/freetype-gl/*.c)
set(SOURCE_FILES ${MAIN_FILES} ${FREETYPE_FILES} grappix/specific/window_pc.cpp)

set(CMAKE_CXX_FLAGS "-std=c++0x -O2")
set(CMAKE_C_FLAGS "-O2")

add_subdirectory(../image image)
add_subdirectory(../tween tween)

if (NOT TARGET coreutils)
	add_subdirectory(../coreutils coreutils)
endif()

if(APPLE)
   find_library(COCOA_LIBRARY Cocoa)
   find_library(CORE_VIDEO_LIBRARY CoreVideo)
   find_library(IOKIT_LIBRARY IOKit)
   set(EXTRA_LIBS ${EXTRA_LIBS} ${COCOA_LIBRARY} ${CORE_VIDEO_LIBRARY} ${IOKIT_LIBRARY})
endif()

find_package(Freetype)
find_package(GLEW)
find_package(OpenGL)

if(FREETYPE_FOUND)
	set(EXTRA_LIBS ${EXTRA_LIBS} ${FREETYPE_LIBRARIES})
	include_directories(${FREETYPE_INCLUDE_DIRS})
	add_definitions(-DWITH_FREETYPE)
endif()

if(GLEW_FOUND)
	set(EXTRA_LIBS ${EXTRA_LIBS} ${GLEW_LIBRARIES})
	include_directories(${GLEW_INCLUDE_DIRS})
endif()

if(OPENGL_FOUND)
	set(EXTRA_LIBS ${EXTRA_LIBS} ${OPENGL_LIBRARIES})
	include_directories(${OPENGL_INCLUDE_DIRS})
endif()

#exec_program(freetype-config ARGS --cflags OUTPUT_VARIABLE FREETYPE_CFLAGS)
#exec_program(freetype-config ARGS --libs OUTPUT_VARIABLE FREETYPE_LIBS)
#find_library(LIBPNG png HINTS ${LIBPNG_LIBDIR})
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FREETYPE_CFLAGS}")
#find_library(FREETYPE_LIBRARY freetype HINT ${FREETYPE_LIBS})
#set(EXTRA_LIBS ${EXTRA_LIBS} ${FREETYPE_LIBS})

find_library(GLFW_LIBRARY glfw3)
set(EXTRA_LIBS ${EXTRA_LIBS} ${GLFW_LIBRARY})

#LIBS += -lglfw3 -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL -lGLEW
set(EXTRA_LIBS ${EXTRA_LIBS} -lX11 -lXxf86vm -lXrandr -lpthread -lXi -lXinerama -lXcursor)

file(GLOB SHADERS grappix/shaders/*.glsl)
file(MAKE_DIRECTORY .shader) 
foreach(SHADER ${SHADERS})
	message(${SHADER})
	get_filename_component(SHADERNAME ${SHADER} NAME)
	add_custom_command(OUTPUT ${SHADERNAME}.c
		COMMAND ${CMAKE_COMMAND} -E copy ${SHADER} .shader/${SHADERNAME}
		COMMAND xxd -i .shader/${SHADERNAME} > ${SHADERNAME}.c 
		DEPENDS ${SHADER})
	set(SOURCE_FILES ${SOURCE_FILES} ${SHADERNAME}.c)
endforeach()

#add_subdirectory(../coreutils coreutils)

include_directories(.. grappix/freetype-gl)

add_library(grappix ${SOURCE_FILES})

target_link_libraries(grappix coreutils image tween GLEW glfw ${EXTRA_LIBS})