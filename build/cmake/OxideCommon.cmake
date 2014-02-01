# vim:expandtab:shiftwidth=2:tabstop=2:

# Copyright (C) 2014 Canonical Ltd.

# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CHROMIUM_BUILD_TYPE Debug)
else()
  set(CHROMIUM_BUILD_TYPE Release)
endif()
set(CHROMIUM_GYP_GENERATOR_DIR ${CMAKE_BINARY_DIR}/gyp/chromium/src)
set(CHROMIUM_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/chromium)
set(CHROMIUM_PRODUCT_DIR ${CHROMIUM_OUTPUT_DIR}/${CHROMIUM_BUILD_TYPE})
set(CHROMIUM_LIB_DIR ${CHROMIUM_PRODUCT_DIR}/lib.target)

if(NOT DEFINED PYTHON)
  find_program(PYTHON python)
  if(PYTHON STREQUAL "PYTHON-NOTFOUND")
    message(FATAL_ERROR "Could not find a python interpreter. Please ensure python is installed")
  endif()
endif()

# Allow the version number to be used in the build
foreach(comp MAJOR MINOR BUILD PATCH)
  execute_process(
      COMMAND ${PYTHON} ${CMAKE_SOURCE_DIR}/build/scripts/get-version.py ${OXIDE_BUILD} ${comp}
      OUTPUT_VARIABLE _OUTPUT
      RESULT_VARIABLE _RESULT)
  if(NOT _RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to get version number")
  endif()
  set(OXIDE_VERSION_${comp} ${_OUTPUT})
endforeach()
