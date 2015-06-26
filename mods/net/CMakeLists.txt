cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.net)

set(SOURCE_FILES net.cpp webgetter.cpp)
include_directories(asio)
add_definitions(-DASIO_STANDALONE)

add_library(net ${SOURCE_FILES})
if(WINDOWS)
	target_link_libraries(net ws2_32)
endif()
