cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.netlink)

set(SOURCE_FILES core.cc smart_buffer.cc socket.cc socket_group.cc util.cc)
include_directories(.)

add_definitions(-Wno-deprecated-register)
#if(WINDOWS)
#	add_definitions(-DOS_WIN32)
#endif()

add_library(netlink ${SOURCE_FILES})
if(WINDOWS)
	target_link_libraries(netlink ws2_32)
endif()
