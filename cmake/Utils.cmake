if(EMSCRIPTEN)

function(SET_DATA_FILES) 
   foreach(DF ${ARGV})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/${DF}.mod@${DF}")
   endforeach()
endfunction()

else()

function(SET_DATA_FILES) 
  foreach(DF ${ARGV})
    get_filename_component(DF_DIR ${DF} DIRECTORY)
    if(DF_DIR)
      file(MAKE_DIRECTORY ${DF_DIR})
    else()
      set(DF_DIR ".")
    endif()
    file(COPY ${DF} DESTINATION ${DF_DIR})
   endforeach()
endfunction()

endif()