if(__GYP_CMAKE_INCLUDED)
  return()
endif()
set(__GYP_CMAKE_INCLUDED TRUE)

if(DEFINED ${CMAKE_BUILD_TYPE} AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(CHROMIUM_BUILD_TYPE Debug)
else()
  set(CHROMIUM_BUILD_TYPE Release)
endif()

set(CHROMIUM_GYP_GENERATOR_DIR ${CMAKE_BINARY_DIR}/chromium/gyp/chromium/src)
set(CHROMIUM_OUTPUT_DIR ${CMAKE_BINARY_DIR}/chromium/out)

set(CHROMIUM_PRODUCT_DIR ${CHROMIUM_OUTPUT_DIR}/${CHROMIUM_BUILD_TYPE})
set(CHROMIUM_LIB_DIR ${CHROMIUM_PRODUCT_DIR}/lib.target)

function(add_gyp_target)
  add_custom_target(
      GYP ALL
      COMMAND ${CMAKE_COMMAND} -DMAKE=$(MAKE) -DBUILDTYPE=${CHROMIUM_BUILD_TYPE}
        -DGYP_DIR=${CHROMIUM_GYP_GENERATOR_DIR}
        -P ${CMAKE_SOURCE_DIR}/build/scripts/gyp-target-shim.cmake
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      COMMENT Building gyp target)
endfunction()
