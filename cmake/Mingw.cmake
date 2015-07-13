# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

set(WHERE /usr)
#set(WHERE /usr/local/gcc-4.8.0-qt-4.8.4-for-mingw32/win32-gcc)
set(CPREFIX x86_64-w64-mingw32-)
#set(CPREFIX i586-mingw32-)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER ${WHERE}/bin/${CPREFIX}gcc)
SET(CMAKE_CXX_COMPILER ${WHERE}/bin/${CPREFIX}g++)
SET(CMAKE_RC_COMPILER ${WHERE}/bin/${CPREFIX}windres)

set(WINROOT /home/sasq/project/apone/sysroots/winroot)
# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH ${WINROOT})

set(CMAKE_C_FLAGS "-DWINDOWS -I${WINROOT}/include" CACHE STRING "")
set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS} CACHE STRING "")
set(CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS})
set(CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS})


SET(WINDOWS 1)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
