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

include(CheckCXXCompilerFlag)

if(NOT DEFINED OXIDE_BUILD)
  set(OXIDE_BUILD qt CACHE INTERNAL "")
endif()

if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CHROMIUM_BUILD_TYPE Debug)
else()
  set(CHROMIUM_BUILD_TYPE Release)
endif()
set(CHROMIUM_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/chromium)
set(CHROMIUM_PRODUCT_DIR ${CHROMIUM_OUTPUT_DIR}/${CHROMIUM_BUILD_TYPE})
set(CHROMIUM_LIB_DIR ${CHROMIUM_PRODUCT_DIR}/lib)

if(ENABLE_COMPONENT_BUILD)
  link_directories(${CHROMIUM_LIB_DIR})
endif()

set(GOLD_DIR ${CMAKE_BINARY_DIR}/gold)

# Use gold everywhere
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -B ${GOLD_DIR}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -B ${GOLD_DIR}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -B ${GOLD_DIR}")

if(NOT DEFINED PYTHON)
  find_program(PYTHON python)
  if(PYTHON STREQUAL "PYTHON-NOTFOUND")
    message(FATAL_ERROR "Could not find a python interpreter. Please ensure python is installed")
  endif()
endif()

# Allow the version number to be used in the build
foreach(comp MAJOR MINOR PATCH)
  execute_process(
      COMMAND ${PYTHON} ${CMAKE_SOURCE_DIR}/build/scripts/get-version.py ${OXIDE_BUILD} ${comp}
      OUTPUT_VARIABLE _OUTPUT
      RESULT_VARIABLE _RESULT)
  if(NOT _RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to get version number")
  endif()
  set(OXIDE_VERSION_${comp} ${_OUTPUT})
endforeach()

# Required for out-of-tree builds
set(CMAKE_INCLUDE_CURRENT_DIR TRUE CACHE INTERNAL "")
# We have a script for loading apps from the tree
set(CMAKE_SKIP_BUILD_RPATH TRUE)
include_directories(${CMAKE_SOURCE_DIR})

if(ENABLE_TESTS)
  enable_testing()
endif()

# for dh_translations to extract the domain
# (regarding syntax consistency, see http://pad.lv/1181187)
set(GETTEXT_PACKAGE "oxide-${OXIDE_BUILD}")

# Import the configuration for this build
include(${CMAKE_SOURCE_DIR}/${OXIDE_BUILD}/config.cmake)

# We want to link the core library with gold. Ubuntu has '-fuse-ld=gold',
# but as this is distro specific, we do something that is currently
# supportable across distributions. This creates a directory with a symlink
# to the gold linker, and we pass -B to the compiler
# The call to "gcc -print-prog-name=ld" is needed so that we get a path
# to the correct linker in cross-builds
execute_process(
    COMMAND ${CMAKE_C_COMPILER} -print-prog-name=ld
    RESULT_VARIABLE _RESULT
    OUTPUT_VARIABLE _LINKER_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT _RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to get linker path from compiler")
endif()
set(_SETUP_GOLD_COMMAND
    ${PYTHON} ${CMAKE_SOURCE_DIR}/build/scripts/setup-gold.py
    --output ${GOLD_DIR} --ld ${_LINKER_PATH})
get_filename_component(_LINKER_NAME ${_LINKER_PATH} NAME)
add_custom_command(
    OUTPUT ${GOLD_DIR}/${_LINKER_NAME}
    COMMAND ${_SETUP_GOLD_COMMAND})
add_custom_target(gold ALL
                  DEPENDS ${GOLD_DIR}/${_LINKER_NAME}
                  COMMENT "Creating a symlink for the gold linker")

# Build the gyp command
set(_GYP_COMMAND
    ${PYTHON} ${CMAKE_SOURCE_DIR}/build/gyp_oxide
    -I${OXIDE_BUILD}/${OXIDE_BUILD}.gypi
    --generator-output ${CHROMIUM_OUTPUT_DIR}
    -Goutput_dir=${CHROMIUM_OUTPUT_DIR}
    ${OXIDE_GYP_EXTRA_ARGS})
if(CMAKE_CROSSCOMPILING)
  if(NOT CHROMIUM_TARGET_ARCH)
    message(FATAL_ERROR "Need to set CHROMIUM_TARGET_ARCH when cross compiling")
  endif()
  list(APPEND _GYP_COMMAND -Dtarget_arch=${CHROMIUM_TARGET_ARCH})
endif()
if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Release")
  list(APPEND _GYP_COMMAND -Dlinux_dump_symbols=0)
else()
  list(APPEND _GYP_COMMAND -Dlinux_dump_symbols=1)
endif()
if(CMAKE_VERBOSE_MAKEFILE)
  list(APPEND _GYP_COMMAND -Dprint_ld_stats=1)
endif()
if(ENABLE_PROPRIETARY_CODECS)
  list(APPEND _GYP_COMMAND -Dffmpeg_branding=Chrome)
  list(APPEND _GYP_COMMAND -Dproprietary_codecs=1)
else()
  list(APPEND _GYP_COMMAND -Dffmpeg_branding=Chromium)
  list(APPEND _GYP_COMMAND -Dproprietary_codecs=0)
endif()
if(DEFINED ENABLE_PLUGINS AND ENABLE_PLUGINS EQUAL 0)
  list(APPEND _GYP_COMMAND -Denable_plugins=0)
else()
  list(APPEND _GYP_COMMAND -Denable_plugins=1)
endif()
if(ENABLE_CHROMEDRIVER)
  list(APPEND _GYP_COMMAND -Denable_chromedriver_build=1)
else()
  list(APPEND _GYP_COMMAND -Denable_chromedriver_build=0)
endif()
if(DEFINED ENABLE_TCMALLOC AND ENABLE_TCMALLOC EQUAL 0)
  list(APPEND _GYP_COMMAND -Denable_tcmalloc=0)
else()
  list(APPEND _GYP_COMMAND -Denable_tcmalloc=1)
endif()
if(ENABLE_COMPONENT_BUILD)
  list(APPEND _GYP_COMMAND -Dcomponent=shared_library)
endif()
list(APPEND _GYP_COMMAND -Doxide_gettext_domain=${GETTEXT_PACKAGE})

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
  set(ENV{PKG_CONFIG_PATH} ${CHROMIUM_PKG_CONFIG_PATH})
else()
  set(ENV{CC} ${CMAKE_C_COMPILER})
  set(ENV{CXX} ${CMAKE_CXX_COMPILER})
  set(ENV{AR} ${CMAKE_AR})
endif()

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_HAS_CXX11)
if(NOT COMPILER_HAS_CXX11)
  message(FATAL_ERROR "Oxide requires a compiler that supports -std=c++11")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

message(STATUS "Running gyp")
execute_process(COMMAND ${_GYP_COMMAND} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE _GYP_RESULT)
if(NOT _GYP_RESULT EQUAL 0)
  message(FATAL_ERROR "Running gyp failed")
endif()

# Add one target for running "ninja all" on the gyp generated bits.
# ${OXIDE_BUILD}/${OXIDE_BUILD}.gypi lists the targets that "all" depends on
find_program(_NINJA_EXE ninja)
if(_NINJA_EXE STREQUAL "_NINJA_EXE-NOTFOUND")
  message(FATAL_ERROR "Could not find ninja, which is required for building Oxide")
endif()
set(_NINJA_BUILD_COMMAND ${_NINJA_EXE} -C ${CHROMIUM_PRODUCT_DIR})
if(CMAKE_VERBOSE_MAKEFILE)
  list(APPEND _NINJA_BUILD_COMMAND -v)
endif()
list(APPEND _NINJA_BUILD_COMMAND all)
add_custom_target(
    gyp_all COMMAND ${_NINJA_BUILD_COMMAND} 
    DEPENDS gold
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building the GYP \"all\" target")

file(GLOB_RECURSE I18N_SRC_FILES
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     ${OXIDE_BUILD}/**.cc shared/**.cc)
list(SORT I18N_SRC_FILES)

add_subdirectory(po)

add_subdirectory(${OXIDE_BUILD})
