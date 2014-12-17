cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods)

foreach(PLUGIN ${MUSICPLAYER_PLUGINS})
    add_subdirectory(${PLUGIN})
endforeach()