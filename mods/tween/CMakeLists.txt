cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.tween)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_library(tween tween.cpp)

if(DO_UNIT_TEST)

	add_definitions(-DUNIT_TEST)
	include_directories(../../test)
	add_executable(tween_test ../../test/catch.cpp)
	set_target_properties(tween_test PROPERTIES LINK_FLAGS "-all_load")
	target_link_libraries(tween_test coreutils tween)
	add_test(tween tween_test)

endif()
