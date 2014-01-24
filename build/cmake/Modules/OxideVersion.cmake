if(__OXIDE_VERSION_CMAKE_INCLUDED)
  return()
endif()
set(__OXIDE_VERSION_CMAKE_INCLUDED TRUE)

foreach(comp MAJOR MINOR BUILD PATCH)
  execute_process(
      COMMAND python ${CMAKE_SOURCE_DIR}/build/scripts/get-version.py ${OXIDE_BUILD} ${comp}
      OUTPUT_VARIABLE _OUTPUT
      RESULT_VARIABLE _RESULT)
  if(NOT ${_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to get version number")
  endif()

  set(OXIDE_VERSION_${comp} ${_OUTPUT})

endforeach()
