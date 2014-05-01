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

if(EXISTS ${CMAKE_BINARY_DIR}/CMakeLists.txt)
  message(FATAL_ERROR
          "In-source-tree builds are unsupported. You must use a separate build "
          "directory to build Oxide. Note that you will need to remove the "
          "CMakeCache.txt file from the root of your source tree before continuing")
endif()

cmake_minimum_required(VERSION 2.8.11)
project(OXIDE)

# This will be an option if we have more than one port
set(OXIDE_BUILD qt CACHE INTERNAL "")

# Required for out-of-tree builds
set(CMAKE_INCLUDE_CURRENT_DIR TRUE CACHE INTERNAL "")

list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/build/cmake
    ${CMAKE_SOURCE_DIR}/${OXIDE_BUILD}/build/cmake)
include(OxideCommon)

if(ENABLE_TESTS)
  enable_testing()
endif()

# Import the configuration for this build
include(${CMAKE_SOURCE_DIR}/${OXIDE_BUILD}/config.cmake)

# We want to link the core library with gold. Ubuntu has '-fuse-ld=gold',
# but as this is distro specific, we do something that is currently
# supportable across distributions. This creates a directory with a symlink
# to the gold linker, and we pass -B to the compiler
execute_process(
    COMMAND ${CMAKE_C_COMPILER} -print-prog-name=ld
    RESULT_VARIABLE _RESULT
    OUTPUT_VARIABLE _LINKER_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT _RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to get linker path from compiler")
endif()
set(SETUP_GOLD_COMMAND
    ${PYTHON} ${CMAKE_SOURCE_DIR}/build/scripts/setup-gold.py
    --output ${CMAKE_BINARY_DIR}/gold --ld ${_LINKER_PATH})
get_filename_component(_LINKER_NAME ${_LINKER_PATH} NAME)
add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/gold/${_LINKER_NAME}
    COMMAND ${SETUP_GOLD_COMMAND})
add_custom_target(gold ALL
                  DEPENDS ${CMAKE_BINARY_DIR}/gold/${_LINKER_NAME}
                  COMMENT "Creating a symlink for gold linker")

message(STATUS "Creating a symlink for gold linker")
execute_process(COMMAND ${SETUP_GOLD_COMMAND}
                RESULT_VARIABLE _RESULT)
if(NOT _RESULT EQUAL 0)
  message(FATAL_ERROR "Couldn't find gold linker. Please check that it is installed")
endif()

# We want to pass extra arguments to our gyp wrapper, but these aren't
# recorded in the gyp regen command in Chromium's top level Makefile.
# To work around it, we preprocess our gyp wrapper with the extra
# arguments embedded
configure_file(
    ${CMAKE_SOURCE_DIR}/build/gyp_oxide.in
    ${CMAKE_BINARY_DIR}/gyp_oxide
    IMMEDIATE @ONLY)
# Build the gyp command
set(GYP_COMMAND
    ${PYTHON} ${CMAKE_BINARY_DIR}/gyp_oxide
    -I${OXIDE_BUILD}/${OXIDE_BUILD}.gypi
    --generator-output ${CHROMIUM_OUTPUT_DIR}
    -Goutput_dir=${CHROMIUM_OUTPUT_DIR}
    ${OXIDE_GYP_EXTRA_ARGS})
if(CMAKE_CROSSCOMPILING)
  if(NOT CHROMIUM_TARGET_ARCH)
    message(FATAL_ERROR "Need to set CHROMIUM_TARGET_ARCH when cross compiling")
  endif()
  list(APPEND GYP_COMMAND -Dtarget_arch=${CHROMIUM_TARGET_ARCH})
endif()
if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Release")
  list(APPEND GYP_COMMAND -Dlinux_dump_symbols=0)
else()
  list(APPEND GYP_COMMAND -Dlinux_dump_symbols=1)
endif()
if(CMAKE_VERBOSE_MAKEFILE)
  list(APPEND GYP_COMMAND -Dprint_ld_stats=1)
endif()
if(ENABLE_PROPRIETARY_CODECS)
  list(APPEND GYP_COMMAND -Dffmpeg_branding=Chrome)
  list(APPEND GYP_COMMAND -Dproprietary_codecs=1)
else()
  list(APPEND GYP_COMMAND -Dffmpeg_branding=Chromium)
  list(APPEND GYP_COMMAND -Dproprietary_codecs=0)
endif()
if(DEFINED ENABLE_PLUGINS AND ENABLE_PLUGINS EQUAL 0)
  list(APPEND GYP_COMMAND -Denable_plugins=0)
else()
  list(APPEND GYP_COMMAND -Denable_plugins=1)
endif()

if(CMAKE_CROSSCOMPILING)
  if(NOT CHROMIUM_C_HOST_COMPILER OR NOT CHROMIUM_CXX_HOST_COMPILER OR NOT CHROMIUM_HOST_AR)
    message(FATAL_ERROR "Need to specify host compilers when cross compiling")
  endif()
  set(ENV{CC_target} ${CMAKE_C_COMPILER})
  set(ENV{CXX_target} ${CMAKE_CXX_COMPILER})
  set(ENV{AR_target} ${CMAKE_AR})
  set(ENV{CC_host} ${CHROMIUM_C_HOST_COMPILER})
  set(ENV{CXX_host} ${CHROMIUM_CXX_HOST_COMPILER})
  set(ENV{AR_host} ${CHROMIUM_HOST_AR})
else()
  set(ENV{CC} ${CMAKE_C_COMPILER})
  set(ENV{CXX} ${CMAKE_CXX_COMPILER})
  set(ENV{AR} ${CMAKE_AR})
endif()

message(STATUS "Running gyp")
execute_process(COMMAND ${GYP_COMMAND} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE _GYP_RESULT)
if(NOT _GYP_RESULT EQUAL 0)
  message(FATAL_ERROR "Running gyp failed")
endif()

# Add one target for running "ninja all" on the gyp generated bits.
# ${OXIDE_BUILD}/${OXIDE_BUILD}.gypi lists the targets that "all" depends on
find_program(NINJA_EXE ninja)
if(NINJA_EXE STREQUAL "NINJA_EXE-NOTFOUND")
  message(FATAL_ERROR "Could not find ninja, which is required for building Oxide")
endif()
set(NINJA_BUILD_COMMAND ${NINJA_EXE} -C ${CHROMIUM_PRODUCT_DIR})
if(CMAKE_VERBOSE_MAKEFILE)
  list(APPEND NINJA_BUILD_COMMAND -v)
endif()
list(APPEND NINJA_BUILD_COMMAND all)
add_custom_target(
    gyp_all ALL COMMAND ${NINJA_BUILD_COMMAND} 
    DEPENDS gold
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building gyp_all target")

add_subdirectory(${OXIDE_BUILD})
