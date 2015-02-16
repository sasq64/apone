cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.coreutils)


set(SOURCE_FILES file.cpp format.cpp log.cpp utils.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")


if(APPLE)
	add_definitions(-DAPPLE)
endif()


if(UNIX)
	add_definitions(-DLINUX)
endif()

add_library(coreutils ${SOURCE_FILES})

if(DO_UNIT_TEST)

	add_definitions(-DUNIT_TEST)
	include_directories(../../test)
	add_executable(coreutils_test ../../test/catch.cpp)
	set_target_properties(coreutils_test PROPERTIES LINK_FLAGS "-all_load")
	target_link_libraries(coreutils_test coreutils)
	add_test(coreutils coreutils_test)

endif()
