# vim:expandtab:shiftwidth=2:tabstop=2:

# Copyright (C) 2016 Canonical Ltd.

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

if(NOT DEFINED PYTHON)
  message(FATAL_ERROR "Must specify a Python binary")
endif()
if(NOT DEFINED CMAKE_SOURCE_DIR)
  message(FATAL_ERROR "Must specify the source directory")
endif()
if(NOT DEFINED CHROMIUM_OUTPUT_DIR)
  message(FATAL_ERROR "Must specify the Chromium output dir")
endif()
if(NOT DEFINED CHROMIUM_PRODUCT_DIR)
  message(FATAL_ERROR "Must specify the Chromium product dir")
endif()
if(NOT DEFINED NINJA_EXE)
  message(FATAL_ERROR "Must specify the Ninja binary")
endif()

if(NOT DEFINED CC)
  message(FATAL_ERROR "Must specify a C compiler")
endif()
if(NOT DEFINED CXX)
  message(FATAL_ERROR "Must specify a C++ compiler")
endif()

set(ENV{CC} ${CC})
set(ENV{CXX} ${CXX})

file(MAKE_DIRECTORY ${CHROMIUM_PRODUCT_DIR}/buildtools)

set(BOOTSTRAP_CMD
    ${PYTHON}
    ${CMAKE_SOURCE_DIR}/tools/gn/bootstrap/bootstrap.py -s
    -o ${CHROMIUM_PRODUCT_DIR}/buildtools/)
if(CMAKE_VERBOSE_MAKEFILE)
  list(APPEND BOOTSTRAP_CMD -v)
endif()
message(STATUS "Building initial Generate Ninja binary")
execute_process(COMMAND ${BOOTSTRAP_CMD}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE BOOTSTRAP_RESULT)
if(NOT BOOTSTRAP_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to bootstrap Generate Ninja binary")
endif()

set(GN_CMD ${CHROMIUM_PRODUCT_DIR}/buildtools/gn gen ${CHROMIUM_OUTPUT_DIR})
message(STATUS "Generating build files with initial Generate Ninja")
execute_process(COMMAND ${GN_CMD}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE GN_RESULT)
if(NOT GN_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to run Generate Ninja")
endif()

set(NINJA_CMD ${NINJA_EXE} -C ${CHROMIUM_OUTPUT_DIR})
if(CMAKE_VERBOSE_MAKEFILE)
  list(APPEND NINJA_CMD -v)
endif()
list(APPEND NINJA_CMD host_gn)
message(STATUS "Building Generate Ninja")
execute_process(COMMAND ${NINJA_CMD}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE NINJA_RESULT)
if(NOT NINJA_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to build Generate Ninja binary")
endif()
