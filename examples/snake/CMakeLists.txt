cmake_minimum_required(VERSION 2.8.4)
project(examples.snake)

add_executable(snake snake.cpp)
target_link_libraries(snake coreutils audioplayer grappix)
