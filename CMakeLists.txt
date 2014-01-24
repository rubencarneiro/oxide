if(EXISTS ${CMAKE_BINARY_DIR}/CMakeLists.txt)
  message(FATAL_ERROR
          "In-source-tree builds are unsupported. You must use a separate build "
          "directory to build Oxide. Note that you will need to remove the "
          "CMakeCache.txt file from the root of your source tree before continuing")
endif()

cmake_minimum_required(VERSION 2.8.11)
project(OXIDE)

include(GNUInstallDirs)

# This will be an option if we have more than one port
set(OXIDE_BUILD qt)

# Required for out-of-tree builds
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

if(DEFINED ${CMAKE_BUILD_TYPE} AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(CHROMIUM_BUILD_TYPE Debug)
else()
  set(CHROMIUM_BUILD_TYPE Release)
endif()
set(CHROMIUM_GYP_GENERATOR_DIR ${CMAKE_BINARY_DIR}/chromium/gyp/chromium/src)
set(CHROMIUM_OUTPUT_DIR ${CMAKE_BINARY_DIR}/chromium/out)
set(CHROMIUM_PRODUCT_DIR ${CHROMIUM_OUTPUT_DIR}/${CHROMIUM_BUILD_TYPE})
set(CHROMIUM_LIB_DIR ${CHROMIUM_PRODUCT_DIR}/lib.target)

# Import the configuration for the port
include(${CMAKE_SOURCE_DIR}/${OXIDE_BUILD}/config.cmake)
if(NOT DEFINED OXIDE_LIB)
  message(FATAL_ERROR "config.make must define OXIDE_LIB")
endif()

# Allow the version number to be used in the build
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

# Add one target for running "make all" on the gyp generated bits.
# ${OXIDE_BUILD}/${OXIDE_BUILD}.gypi lists the targets that "all" depends on
add_custom_target(
    gyp_all ALL
    COMMAND ${CMAKE_COMMAND} -DMAKE=$(MAKE) -DBUILDTYPE=${CHROMIUM_BUILD_TYPE}
      -DGYP_DIR=${CHROMIUM_GYP_GENERATOR_DIR}
      -P ${CMAKE_SOURCE_DIR}/build/scripts/gyp-target-shim.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building gyp_all target")

# Allow objects linked by cmake to declare a run-time dependency on the
# main library, built with gyp
add_library(OxideLib SHARED IMPORTED)
set(OXIDE_LIB ${CHROMIUM_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${OXIDE_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX})
set_target_properties(OxideLib PROPERTIES IMPORTED_LOCATION ${OXIDE_LIB})

add_subdirectory(${OXIDE_BUILD})
