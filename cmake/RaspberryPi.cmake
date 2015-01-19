# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

#SET(PREF arm-linux-gnueabihf-)
#SET(PI_SDK_ROOT /opt/raspberry/arm-bcm2708/gcc-linaro-${PREF}raspbian)
SET(PREF arm-none-linux-gnueabi-)
SET(PI_SDK_ROOT /opt/raspberry/arm-none-linux-gnueabi)

GET_FILENAME_COMPONENT(PI_SYS_ROOT ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
GET_FILENAME_COMPONENT(PI_SYS_ROOT "${PI_SYS_ROOT}/sysroots/piroot" ABSOLUTE)
message(${PI_SYS_ROOT})

# specify the cross compiler
SET(CMAKE_C_COMPILER ${PI_SDK_ROOT}/bin/${PREF}gcc)

SET(CMAKE_CXX_COMPILER ${PI_SDK_ROOT}/bin/${PREF}g++)

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH ${PI_SYS_ROOT})

SET(CMAKE_C_FLAGS "-DGL_ES -DRASPBERRYPI -DLINUX -march=armv6 -mfpu=vfp -mfloat-abi=hard -I${PI_SYS_ROOT}/include -I${PI_SYS_ROOT}/vc/include")
SET(CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS})
SET(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS})
SET(CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS})
SET(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS})

SET(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc -L${PI_SYS_ROOT}/lib -L${PI_SYS_ROOT}/vc/lib")

SET(RASPBERRY 1)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
