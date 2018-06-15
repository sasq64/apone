cmake_minimum_required(VERSION 3.1.0)
project(chipmachine)

if(MSVC) 
	set(STDFLAG /std:c++17)
	set(FSLIB "")
elseif(APPLE)
	set(STDFLAG -std=c++17)
	set(FSLIB c++experimental)
else()
    set(STDFLAG -std=c++17)
	set(FSLIB stdc++fs)
endif()

add_executable(testing mods/coreutils/newfile.t.cpp mods/luainterpreter/luainterpreter.t.cpp mods/luainterpreter/luainterpreter.cpp)
target_link_libraries(testing ${FSLIB} pthread)
target_include_directories(testing PRIVATE mods)
target_compile_definitions(testing PRIVATE -DTESTING)

