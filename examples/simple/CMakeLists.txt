cmake_minimum_required(VERSION 2.8.4)
project(examples.simple)

add_executable(simple simple.cpp)
target_link_libraries(simple coreutils grappix)
