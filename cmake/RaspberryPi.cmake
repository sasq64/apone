# this one is important
set(CMAKE_SYSTEM_NAME Linux)
#this one not so much
set(CMAKE_SYSTEM_VERSION 1)

set(PLATFORMS arm-linux-gnueabihf 
#set(PREF arm-linux-gnueabihf-)
#set(PI_SDK_ROOT /opt/raspberry/arm-bcm2708/gcc-linaro-${PREF}raspbian)
set(PREF arm-none-linux-gnueabi-)
set(PI_SDK_ROOT /opt/raspberry/arm-none-linux-gnueabi)

get_filename_component(PI_SYS_ROOT ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
get_filename_component(PI_SYS_ROOT "${PI_SYS_ROOT}/sysroots/piroot" ABSOLUTE)
message(${PI_SYS_ROOT})

# specify the cross compiler
set(CMAKE_C_COMPILER ${PI_SDK_ROOT}/bin/${PREF}gcc)

set(CMAKE_CXX_COMPILER ${PI_SDK_ROOT}/bin/${PREF}g++)

# where is the target environment
set(CMAKE_FIND_ROOT_PATH ${PI_SYS_ROOT})

set(CMAKE_C_FLAGS "-DGL_ES -DRASPBERRYPI -DLINUX -march=armv6 -mfpu=vfp -mfloat-abi=hard -I${PI_SYS_ROOT}/include -I${PI_SYS_ROOT}/vc/include")
set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
set(CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS})
set(CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS})
set(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS})

set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc -L${PI_SYS_ROOT}/lib -L${PI_SYS_ROOT}/vc/lib")

set(RASPBERRY 1)

# search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
