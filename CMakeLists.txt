# vim:expandtab:shiftwidth=2:tabstop=2:

# Copyright (C) 2014-2016 Canonical Ltd.

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
include(CheckCXXSymbolExists)
include(CheckIncludeFileCXX)
include(CMakeExpandImportedTargets)
include(CMakeParseArguments)
include(GNUInstallDirs)

if(NOT DEFINED OXIDE_BUILD)
  set(OXIDE_BUILD qt CACHE INTERNAL "")
endif()

# Define variables used throughout the build
if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CHROMIUM_BUILD_TYPE Debug)
else()
  set(CHROMIUM_BUILD_TYPE Release)
endif()
set(CHROMIUM_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/chromium)
set(CHROMIUM_PRODUCT_DIR ${CHROMIUM_OUTPUT_DIR}/${CHROMIUM_BUILD_TYPE})
set(CHROMIUM_LIB_DIR ${CHROMIUM_PRODUCT_DIR}/lib)
set(OXIDE_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out)
set(OXIDE_BIN_OUTPUT_DIR ${OXIDE_OUTPUT_DIR}/bin)
set(OXIDE_LIB_OUTPUT_DIR ${OXIDE_OUTPUT_DIR}/lib)
set(OXIDE_PACKAGE_STAGE_DIR ${CMAKE_BINARY_DIR}/_package_stage)

# Use gold everywhere
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fuse-ld=gold")

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
      RESULT_VARIABLE _RESULT
      OUTPUT_STRIP_TRAILING_WHITESPACE)
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

option(OXIDE_BUILD "The Oxide project to build" qt)
option(ENABLE_COMPONENT_BUILD
       "Build all components of the core library as shared objects"
       OFF)
option(ENABLE_TESTS "Enable the tests" OFF)
option(ENABLE_PROPRIETARY_CODECS "Enable support for MP3, H.264 and AAC" OFF)
option(ENABLE_PLUGINS
       "Enable support for PPAPI plugins. Only Flash is supported right now"
       ON)
option(ENABLE_CHROMEDRIVER "Build the chromedriver binary" OFF)
option(ENABLE_TCMALLOC "Use TCMalloc in the renderer executable" ON)
option(USE_SYSTEM_PROTOBUF "Use the system protobuf" OFF)
option(ENABLE_HYBRIS "Enable code that uses libhybris" ON)
option(ENABLE_HYBRIS_CAMERA
       "Enable support for the camera compatibility layer in Ubuntu's libhybris"
       OFF)

if(ENABLE_HYBRIS_CAMERA AND NOT ENABLE_HYBRIS)
  message(FATAL_ERROR "ENABLE_HYBRIS_CAMERA requires ENABLE_HYBRIS")
endif()

# for dh_translations to extract the domain
# (regarding syntax consistency, see http://pad.lv/1181187)
set(GETTEXT_PACKAGE "oxide-${OXIDE_BUILD}")

if(ENABLE_COMPONENT_BUILD)
  link_directories(${CHROMIUM_LIB_DIR})
endif()

if(ENABLE_TESTS)
  enable_testing()
endif()

# Import the configuration for this build
include(${CMAKE_SOURCE_DIR}/${OXIDE_BUILD}/config.cmake)

set(OXIDE_LIB_FILENAME_DEV
    ${CMAKE_SHARED_LIBRARY_PREFIX}${OXIDE_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX})
set(OXIDE_LIB_FILENAME ${OXIDE_LIB_FILENAME_DEV}.${OXIDE_LIB_VERSION})
set(CMAKE_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_LIBDIR}/${OXIDE_PROJECT_NAME})

# Check we have a new enough compiler
# FIXME: Support clang
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
  message(FATAL_ERROR "Oxide requires at least GCC 4.8 to build")
endif()
CHECK_CXX_COMPILER_FLAG("-std=c++11" _COMPILER_HAS_CXX11_RESULT)
if(NOT _COMPILER_HAS_CXX11_RESULT)
  message(FATAL_ERROR "Oxide requires a compiler that supports -std=c++11")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

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
    build_all ALL COMMAND ${_NINJA_BUILD_COMMAND}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running the build_all target")

# Copy chrome_sandbox to chrome-sandbox
add_custom_target(
    copy_chrome_sandbox ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CHROMIUM_PRODUCT_DIR}/chrome_sandbox ${CHROMIUM_PRODUCT_DIR}/chrome-sandbox
    DEPENDS build_all)

# Allow objects linked by cmake to declare a run-time dependency on the
# main library, built with gyp
add_library(${OXIDE_LIB} SHARED IMPORTED)
add_dependencies(${OXIDE_LIB} build_all)
set_target_properties(
    ${OXIDE_LIB} PROPERTIES
    IMPORTED_LOCATION ${CHROMIUM_LIB_DIR}/${OXIDE_LIB_FILENAME})

file(GLOB_RECURSE I18N_SRC_FILES
     RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
     ${OXIDE_BUILD}/**.cc shared/**.cc)
list(SORT I18N_SRC_FILES)

set(_OXIDE_INSTALL_LIB ${CHROMIUM_LIB_DIR}/${OXIDE_LIB_FILENAME})
set(_OXIDE_INSTALL_FILES
    ${CHROMIUM_PRODUCT_DIR}/oxide.pak
    ${CHROMIUM_PRODUCT_DIR}/oxide_100_percent.pak
    ${CHROMIUM_PRODUCT_DIR}/oxide_200_percent.pak
    ${CHROMIUM_PRODUCT_DIR}/icudtl.dat
    ${CHROMIUM_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}ffmpeg${CMAKE_SHARED_LIBRARY_SUFFIX}
    ${CHROMIUM_PRODUCT_DIR}/natives_blob.bin
    ${CHROMIUM_PRODUCT_DIR}/snapshot_blob.bin)
set(_OXIDE_INSTALL_PROGRAMS ${CHROMIUM_PRODUCT_DIR}/${OXIDE_RENDERER})
if(ENABLE_CHROMEDRIVER)
  list(APPEND _OXIDE_INSTALL_PROGRAMS ${CHROMIUM_PRODUCT_DIR}/chromedriver)
endif()
set(_CHROME_SANDBOX ${CHROMIUM_PRODUCT_DIR}/chrome-sandbox)
set(_OXIDE_INSTALL_LOCALEDIR ${CHROMIUM_PRODUCT_DIR}/locales)

install(FILES ${_OXIDE_INSTALL_LIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(CODE
        "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${OXIDE_LIB_FILENAME} ${OXIDE_LIB_FILENAME_DEV}
        WORKING_DIRECTORY \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})")
install(FILES ${_OXIDE_INSTALL_FILES} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(PROGRAMS ${_OXIDE_INSTALL_PROGRAMS}
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(PROGRAMS ${_CHROME_SANDBOX}
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE
                    GROUP_READ WORLD_EXECUTE WORLD_READ SETUID)
install(DIRECTORY ${_OXIDE_INSTALL_LOCALEDIR}
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
if(ENABLE_COMPONENT_BUILD)
  install(DIRECTORY ${CHROMIUM_LIB_DIR}
          DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}
          FILES_MATCHING PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif()

add_subdirectory(po)
add_subdirectory(${OXIDE_BUILD})

add_custom_target(
    test-package-common-libs
    COMMAND ${CMAKE_COMMAND} -DFILES=${_OXIDE_INSTALL_LIB}
    -DTYPE=FILE -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}
    -P ${CMAKE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)
add_custom_target(
    test-package-common-programs
    COMMAND ${CMAKE_COMMAND}
    "-DFILES=\"${_OXIDE_INSTALL_PROGRAMS};${_CHROME_SANDBOX}\""
    -DTYPE=PROGRAM
    -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}/${OXIDE_PROJECT_NAME}
    -P ${CMAKE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)
add_custom_target(
    test-package-common-files
    COMMAND ${CMAKE_COMMAND} "-DFILES=\"${_OXIDE_INSTALL_FILES}\""
    -DTYPE=FILE
    -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}/${OXIDE_PROJECT_NAME}
    -P ${CMAKE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)
add_custom_target(
    test-package-common-locales
    COMMAND ${CMAKE_COMMAND} -DFILES=${_OXIDE_INSTALL_LOCALEDIR}
    -DTYPE=DIRECTORY
    -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}/${OXIDE_PROJECT_NAME}
    -P ${CMAKE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)
if(ENABLE_COMPONENT_BUILD)
  add_custom_target(
      test-package-common-component-libs
      COMMAND ${CMAKE_COMMAND} -DFILES=${_OXIDE_INSTALL_LOCALEDIR}
      -DMATCH_PATTERN="*${CMAKE_SHARED_LIBRARY_SUFFIX}" -DTYPE=DIRECTORY
      -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}/${OXIDE_PROJECT_NAME}
      -P ${CMAKE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)
endif()

add_custom_target(
    test-package-common
    DEPENDS test-package-common-libs
            test-package-common-programs
            test-package-common-files
            test-package-common-locales)
if(ENABLE_COMPONENT_BUILD)
  add_dependencies(test-package-common test-package-common-component-libs)
endif()

add_custom_target(
    test-package
    COMMAND ${CMAKE_COMMAND} -E
    tar czvf ${OXIDE_OUTPUT_DIR}/${OXIDE_PROJECT_NAME}.tar.bz2 *
    WORKING_DIRECTORY ${OXIDE_PACKAGE_STAGE_DIR}
    DEPENDS test-package-common test-package-${OXIDE_BUILD})

# Build the gyp command
set(_GYP_COMMAND
    ${PYTHON} ${CMAKE_SOURCE_DIR}/build/gyp_oxide
    -I${OXIDE_BUILD}/${OXIDE_BUILD}.gypi
    --generator-output ${CHROMIUM_OUTPUT_DIR}
    -Goutput_dir=${CHROMIUM_OUTPUT_DIR}
    ${OXIDE_GYP_EXTRA_ARGS})
list(APPEND _GYP_COMMAND -Doxide_lib=${OXIDE_LIB})
list(APPEND _GYP_COMMAND -Doxide_lib_version=${OXIDE_LIB_VERSION})
list(APPEND _GYP_COMMAND -Doxide_renderer=${OXIDE_RENDERER})
list(APPEND _GYP_COMMAND -Doxide_libexecdir=${OXIDE_PROJECT_NAME})
list(APPEND _GYP_COMMAND -Doxide_build=${OXIDE_BUILD})
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
if(ENABLE_PLUGINS)
  list(APPEND _GYP_COMMAND -Denable_plugins=1)
else()
  list(APPEND _GYP_COMMAND -Denable_plugins=0)
endif()
if(ENABLE_CHROMEDRIVER)
  list(APPEND _GYP_COMMAND -Denable_chromedriver_build=1)
else()
  list(APPEND _GYP_COMMAND -Denable_chromedriver_build=0)
endif()
if(ENABLE_TCMALLOC)
  list(APPEND _GYP_COMMAND -Denable_tcmalloc=1)
else()
  list(APPEND _GYP_COMMAND -Denable_tcmalloc=0)
endif()
if(ENABLE_COMPONENT_BUILD)
  list(APPEND _GYP_COMMAND -Dcomponent=shared_library)
endif()
list(APPEND _GYP_COMMAND -Doxide_gettext_domain=${GETTEXT_PACKAGE})
CHECK_INCLUDE_FILE_CXX("core/media/player.h" HAVE_MEDIAHUB "--std=c++11")
if(NOT HAVE_MEDIAHUB)
  list(APPEND _GYP_COMMAND -Denable_mediahub=0)
else()
  list(APPEND _GYP_COMMAND -Denable_mediahub=1)
endif()
if(USE_SYSTEM_PROTOBUF)
  list(APPEND _GYP_COMMAND -Duse_system_protobuf=1)
endif()
if(ENABLE_HYBRIS)
  list(APPEND _GYP_COMMAND -Denable_hybris=1)
else()
  list(APPEND _GYP_COMMAND -Denable_hybris=0)
endif()
if(ENABLE_HYBRIS_CAMERA)
  list(APPEND _GYP_COMMAND -Denable_hybris_camera=1)
else()
  list(APPEND _GYP_COMMAND -Denable_hybris_camera=0)
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
  set(ENV{PKG_CONFIG_PATH} ${CHROMIUM_PKG_CONFIG_PATH})
else()
  set(ENV{CC} ${CMAKE_C_COMPILER})
  set(ENV{CXX} ${CMAKE_CXX_COMPILER})
  set(ENV{AR} ${CMAKE_AR})
endif()

message(STATUS "Running gyp")
execute_process(COMMAND ${_GYP_COMMAND} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE _GYP_RESULT)
if(NOT _GYP_RESULT EQUAL 0)
  message(FATAL_ERROR "Running gyp failed")
endif()
