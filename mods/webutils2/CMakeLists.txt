cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.webutils2)

set(SOURCE_FILES web.cpp)

find_library(CURL_LIBRARY curl REQUIRED)
find_path(CURL_INCLUDE curl/curl.h)

include_directories(${CURL_INCLUDE})

add_library(webutils2 ${SOURCE_FILES})
target_link_libraries(webutils2 pthread ${CURL_LIBRARY})
