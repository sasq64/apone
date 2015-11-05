cmake_minimum_required(VERSION 2.8.4)
project(cpp-mods.webutils)

set(SOURCE_FILES web.cpp webgetter.cpp webrpc.cpp)

find_library(CURL_LIBRARY curl REQUIRED)
find_path(CURL_INCLUDE curl/curl.h)
#set(CURL_LIBRARY curl)

include_directories(${CURL_INCLUDE})

add_library(webutils ${SOURCE_FILES})
target_link_libraries(webutils pthread ${CURL_LIBRARY})
