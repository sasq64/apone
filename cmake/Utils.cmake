
if(EXISTS /opt/vc/include/bcm_host.h)
  message("DETECTED RASPBERRY PI")
  set(RASPBERRY 1)
  set(PI_SYS_ROOT /opt)
  set(RPI_FLAGS "-DGL_ES -DRASPBERRYPI -mfloat-abi=hard -I/opt/vc/include")
  exec_program(uname ARGS -m OUTPUT_VARIABLE RPI_ARCH)
  if(RPI_ARCH MATCHES "armv7l")
    message("v2 neon detected")
    set(RPI_FLAGS "${RPI_FLAGS} -mfpu=neon")
  else()
    set(RPI_FLAGS "${RPI_FLAGS} -mfpu=vfp")
  endif()

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${RPI_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RPI_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/opt/vc/lib")
endif() 


if(EMSCRIPTEN)

include_directories(../mods/zlib/zlib-1.2.8)

macro(SET_DATA_FILES)
  message("ARGV " ${ARGV})
   foreach(DF ${ARGV})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/${DF}@${DF}")
   endforeach()
   message("FLAGS " ${CMAKE_EXE_LINKER_FLAGS})
endmacro()


else()

function(SET_DATA_FILES) 
  foreach(DF ${ARGV})
    get_filename_component(DF_DIR ${DF} PATH)
    if(DF_DIR)
      file(MAKE_DIRECTORY ${DF_DIR})
    else()
      set(DF_DIR ".")
    endif()
    file(COPY ${DF} DESTINATION ${DF_DIR})
   endforeach()
endfunction()

endif()