cmake_minimum_required(VERSION 2.8.11)
project(OXIDE)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/build/cmake/Modules)
set(OXIDE_BUILD qt)

include(${CMAKE_SOURCE_DIR}/${OXIDE_BUILD}/config.cmake)
include(OxideGyp)

message(STATUS "Running gyp")
execute_process(
    COMMAND ${CMAKE_SOURCE_DIR}/build/gyp_oxide
      -I${OXIDE_BUILD}/${OXIDE_BUILD}.gypi
      --output-dir ${CHROMIUM_OUTPUT_DIR}
      --generator-output ${CHROMIUM_GYP_GENERATOR_DIR}
      ${OXIDE_GYP_EXTRA_ARGS}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE _GYP_RESULT)

if(NOT ${_GYP_RESULT} EQUAL 0)
  message(FATAL_ERROR "Running gyp failed")
endif()

add_subdirectory(${OXIDE_BUILD})
