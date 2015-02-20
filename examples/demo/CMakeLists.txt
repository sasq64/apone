cmake_minimum_required(VERSION 2.8.4)
project(examples.demo)

add_subdirectory(${AP1_ROOT}/mods/musicplayer/plugins/openmptplugin openmptplugin)

SET_DATA_FILES(data/stardust.mod data/ObelixPro.ttf)

add_executable(demo demo.cpp)
target_link_libraries(demo coreutils audioplayer grappix openmptplugin)
