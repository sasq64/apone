cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.tween)



add_library(tween tween.cpp)
target_include_directories(tween PRIVATE ..)

if(DO_UNIT_TEST)

	add_definitions(-DUNIT_TEST)
	include_directories(../../test)
	add_executable(tween_test ../../test/catch.cpp)
	set_target_properties(tween_test PROPERTIES LINK_FLAGS "-all_load")
	target_link_libraries(tween_test coreutils tween)
	add_test(tween tween_test)

endif()
