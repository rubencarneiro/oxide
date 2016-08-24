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

if(DEFINED _Oxide_ChromiumBuildShim_INCLUDED_)
  return()
endif()
set(_Oxide_ChromiumBuildShim_INCLUDED_ TRUE)

include(CMakeParseArguments)
include(CommonOptions)
include(CommonProperties)
include(LibFilenameUtils)

find_program(NINJA_EXE ninja)
if(NINJA_EXE STREQUAL "NINJA_EXE-NOTFOUND")
  message(FATAL_ERROR "Could not find ninja, which is required for building Oxide")
endif()

if(BOOTSTRAP_GN)
  set(GN_EXE ${CHROMIUM_PRODUCT_DIR}/buildtools/gn)
else()
  find_program(GN_EXE gn)
  if(GN_EXE STREQUAL "GN_EXE-NOTFOUND")
    message(FATAL_ERROR
            "Can't find gn binary. Did you check out the source following "
            "the instructions at https://wiki.ubuntu.com/Oxide/GetTheCode?")
  endif()
endif()

macro(_bootstrap_gn)
  if(NOT EXISTS "${CHROMIUM_PRODUCT_DIR}/buildtools/gn")
    set(BOOTSTRAP_CMD ${CMAKE_COMMAND}
        -DPYTHON=${PYTHON} -DCMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}
        -DCHROMIUM_OUTPUT_DIR=${CHROMIUM_OUTPUT_DIR}
        -DCHROMIUM_PRODUCT_DIR=${CHROMIUM_PRODUCT_DIR}
        -DNINJA_EXE=${NINJA_EXE}
        -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE})
    if(CMAKE_CROSSCOMPILING)
      list(APPEND BOOTSTRAP_CMD -DCC=${CHROMIUM_C_HOST_COMPILER})
      list(APPEND BOOTSTRAP_CMD -DCXX=${CHROMIUM_CXX_HOST_COMPILER})
    else()
      list(APPEND BOOTSTRAP_CMD -DCC=${CMAKE_C_COMPILER})
      list(APPEND BOOTSTRAP_CMD -DCXX=${CMAKE_CXX_COMPILER})
    endif()
    list(APPEND BOOTSTRAP_CMD -P ${OXIDE_SOURCE_DIR}/build/scripts/bootstrap-gn.cmake)
    message(STATUS "Bootstrapping Generate Ninja binary")
    execute_process(COMMAND ${BOOTSTRAP_CMD}
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    RESULT_VARIABLE BOOTSTRAP_RESULT)
    if(NOT BOOTSTRAP_RESULT EQUAL 0)
      message(FATAL_ERROR "Failed to bootstrap Generate Ninja binary")
    endif()
  endif()
endmacro()

macro(_determine_compiler _out _compiler_id)
  if (${_compiler_id} STREQUAL "GNU")
    set(${_out} "gcc")
  else()
    message(FATAL_ERROR "Unrecognized compiler '${_compiler_id}'")
  endif()
endmacro()

function(add_chromium_build_all_target name)
  set(SKIP_GN_CHECK True) # https://launchpad.net/bugs/1594962
  if(NOT ENABLE_PLUGINS)
    # There are failures in //chrome with enable_plugins=false
    set(SKIP_GN_CHECK true)
  endif()

  set(GN_CHECK_TARGET)
  if(NOT SKIP_GN_CHECK)
    set(GN_CHECK_TARGET ${name}_gncheck)
    add_custom_target(
        ${GN_CHECK_TARGET} ALL
        COMMAND ${GN_EXE} check ${CHROMIUM_OUTPUT_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running gn check")
  endif()

  set(NINJA_CMD ${NINJA_EXE} -C ${CHROMIUM_OUTPUT_DIR})
  if(CMAKE_VERBOSE_MAKEFILE)
    list(APPEND NINJA_CMD -v)
  endif()
  list(APPEND NINJA_CMD oxide_all)
  add_custom_target(
      ${name} ALL COMMAND ${NINJA_CMD}
      DEPENDS ${GNCHECK_TARGET}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      COMMENT "Running the Chromium build all target")
endfunction()

function(run_generate_ninja)
  if(NOT DEFINED OXIDE_LIB OR
     NOT DEFINED OXIDE_LIB_VERSION OR
     NOT DEFINED OXIDE_RENDERER)
    message(FATAL_ERROR "One or more parameters are missing from the build")
  endif()

  set(MAKE_GN_ARGS_CMD
      ${PYTHON} ${OXIDE_SOURCE_DIR}/build/scripts/make_gn_args.py)

  if(ENABLE_COMPONENT_BUILD)
    list(APPEND MAKE_GN_ARGS_CMD --component-build)
  endif()

  if(NOT DEFINED CMAKE_BUILD_TYPE OR NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND MAKE_GN_ARGS_CMD --enable-debug-symbols)
  endif()

  list(APPEND MAKE_GN_ARGS_CMD --output-dir ${CHROMIUM_OUTPUT_DIR})

  if(NOT CMAKE_CROSSCOMPILING)
    _determine_compiler(HOST_COMPILER ${CMAKE_CXX_COMPILER_ID})
    list(APPEND MAKE_GN_ARGS_CMD -Doxide_host_ar=${CMAKE_AR})
    list(APPEND MAKE_GN_ARGS_CMD -Doxide_host_cc=${CMAKE_C_COMPILER})
    list(APPEND MAKE_GN_ARGS_CMD -Doxide_host_cxx=${CMAKE_CXX_COMPILER})
  else()
    if(NOT CHROMIUM_C_HOST_COMPILER OR NOT CHROMIUM_CXX_HOST_COMPILER OR
       NOT CHROMIUM_HOST_AR OR NOT CHROMIUM_CXX_HOST_COMPILER_ID)
      message(FATAL_ERROR "Your toolchain config must define a host toolchain")
    endif()

    if(NOT CMAKE_SYSTEM_NAME STREQUAL CMAKE_HOST_SYSTEM_NAME)
      message(FATAL_ERROR "We don't support cross-compiling for a different system")
    endif()

    if(NOT DEFINED CHROMIUM_TARGET_ARCH)
      message(FATAL_ERROR "Your toolchain config must define CHROMIUM_TARGET_ARCH")
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT DEFINED CHROMIUM_PKG_CONFIG)
      message(FATAL_ERROR "Your toolchain config must defined CHROMIUM_PKG_CONFIG")
    endif()

    _determine_compiler(HOST_COMPILER ${CHROMIUM_CXX_HOST_COMPILER_ID})
    _determine_compiler(TARGET_COMPILER ${CMAKE_CXX_COMPILER_ID})

    list(APPEND MAKE_GN_ARGS_CMD -Doxide_host_ar=${CHROMIUM_HOST_AR})
    list(APPEND MAKE_GN_ARGS_CMD -Doxide_host_cc=${CHROMIUM_C_HOST_COMPILER})
    list(APPEND MAKE_GN_ARGS_CMD -Doxide_host_cxx=${CHROMIUM_CXX_HOST_COMPILER})

    list(APPEND MAKE_GN_ARGS_CMD -Doxide_cross_ar=${CMAKE_AR})
    list(APPEND MAKE_GN_ARGS_CMD -Doxide_cross_cc=${CMAKE_C_COMPILER})
    list(APPEND MAKE_GN_ARGS_CMD -Doxide_cross_cxx=${CMAKE_CXX_COMPILER})

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      if(NOT DEFINED CHROMIUM_NM OR NOT DEFINED CHROMIUM_READELF)
        message(FATAL_ERROR "Your toolchain config must define CHROMIUM_NM and CHROMIUM_READELF")
      endif()
      list(APPEND MAKE_GN_ARGS_CMD -Doxide_cross_nm=${CHROMIUM_NM})
      list(APPEND MAKE_GN_ARGS_CMD -Doxide_cross_readelf=${CHROMIUM_READELF})
    endif()

    list(APPEND MAKE_GN_ARGS_CMD --target-arch ${CHROMIUM_TARGET_ARCH})
    list(APPEND MAKE_GN_ARGS_CMD --target-compiler ${TARGET_COMPILER})

    list(APPEND MAKE_GN_ARGS_CMD -Dpkg_config=${CHROMIUM_PKG_CONFIG})
  endif()
  list(APPEND MAKE_GN_ARGS_CMD --host-compiler ${HOST_COMPILER})

  if(DEFINED ENV{CFLAGS})
    list(APPEND MAKE_GN_ARGS_CMD -Doxide_extra_cflags=${ENV{CFLAGS}})
  endif()
  if(DEFINED ENV{CPPFLAGS})
    list(APPEND MAKE_GN_ARGS_CMD -Doxide_extra_cppflags=${ENV{CPPFLAGS}})
  endif()
  if(DEFINED ENV{CXXFLAGS})
    list(APPEND MAKE_GN_ARGS_CMD -Doxide_extra_cxxflags=${ENV{CXXFLAGS}})
  endif()
  if(DEFINED ENV{LDFLAGS})
    list(APPEND MAKE_GN_ARGS_CMD -Doxide_extra_ldflags=${ENV{LDFLAGS}})
  endif()

  get_library_extension(LIB_EXTENSION ${OXIDE_LIB_VERSION})
  string(REGEX REPLACE "^\\." "" LIB_EXTENSION ${LIB_EXTENSION})
  list(APPEND MAKE_GN_ARGS_CMD -Doxide_libexec_subdir=${OXIDE_PLATFORM_FULLNAME})
  list(APPEND MAKE_GN_ARGS_CMD -Doxide_lib_extension=${LIB_EXTENSION})
  list(APPEND MAKE_GN_ARGS_CMD -Doxide_lib_name=${OXIDE_LIB})
  list(APPEND MAKE_GN_ARGS_CMD -Doxide_platform=${OXIDE_PLATFORM})
  list(APPEND MAKE_GN_ARGS_CMD -Doxide_renderer_name=${OXIDE_RENDERER})

  if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND MAKE_GN_ARGS_CMD -Dis_debug=true)
  else()
    list(APPEND MAKE_GN_ARGS_CMD -Dis_debug=false)
  endif()

  if(CMAKE_VERBOSE_MAKEFILE)
    list(APPEND GN_COMMAND -Dprint_gold_stats=true)
  endif()

  if(ENABLE_PROPRIETARY_CODECS)
    list(APPEND MAKE_GN_ARGS_CMD -Dffmpeg_branding=Chrome)
    list(APPEND MAKE_GN_ARGS_CMD -Dproprietary_codecs=true)
  else()
    list(APPEND MAKE_GN_ARGS_CMD -Dffmpeg_branding=Chromium)
    list(APPEND MAKE_GN_ARGS_CMD -Dproprietary_codecs=false)
  endif()

  if(ENABLE_PLUGINS)
    list(APPEND MAKE_GN_ARGS_CMD -Denable_plugins=true)
  else()
    list(APPEND MAKE_GN_ARGS_CMD -Denable_plugins=false)
  endif()

  if(ENABLE_TCMALLOC)
    list(APPEND MAKE_GN_ARGS_CMD -Denable_tcmalloc=true)
  else()
    list(APPEND MAKE_GN_ARGS_CMD -Denable_tcmalloc=false)
  endif()

  list(APPEND MAKE_GN_ARGS_CMD -Doxide_gettext_domain=${GETTEXT_PACKAGE})

  check_include_file_cxx("core/media/player.h" HAVE_MEDIAHUB "--std=c++11")
  if(NOT HAVE_MEDIAHUB)
    list(APPEND MAKE_GN_ARGS_CMD -Denable_mediahub=false)
  else()
    list(APPEND MAKE_GN_ARGS_CMD -Denable_mediahub=true)
  endif()

  if(USE_SYSTEM_PROTOBUF)
    message(FATAL_ERROR "USE_SYSTEM_PROTOBUF is currently unsupported")
  endif()

  if(ENABLE_HYBRIS)
    list(APPEND MAKE_GN_ARGS_CMD -Denable_hybris=true)
  else()
    list(APPEND MAKE_GN_ARGS_CMD -Denable_hybris=false)
  endif()

  if(ENABLE_HYBRIS_CAMERA)
    list(APPEND MAKE_GN_ARGS_CMD -Denable_hybris_camera=true)
  else()
    list(APPEND MAKE_GN_ARGS_CMD -Denable_hybris_camera=false)
  endif()

  if(ENABLE_TESTS)
    list(APPEND MAKE_GN_ARGS_CMD -Denable_tests=true)
    if(ENABLE_CHROMIUM_TESTS)
      list(APPEND MAKE_GN_ARGS_CMD -Denable_chromium_tests=true)
    else()
      list(APPEND MAKE_GN_ARGS_CMD -Denable_chromium_tests=false)
    endif()
  else()
    list(APPEND MAKE_GN_ARGS_CMD -Denable_tests=false)
    list(APPEND MAKE_GN_ARGS_CMD -Denable_chromium_tests=false)
  endif()

  if(DEFINED QT_MOC_EXECUTABLE)
    list(APPEND MAKE_GN_ARGS_CMD -Dqt_moc_executable=${QT_MOC_EXECUTABLE})
  endif()

  if(BOOTSTRAP_GN)
    list(APPEND MAKE_GN_ARGS_CMD -Denable_gn_build=true)
  else()
    list(APPEND MAKE_GN_ARGS_CMD -Denable_gn_build=false)
  endif()

  message(STATUS "Writing Generate Ninja configuration")
  execute_process(COMMAND ${MAKE_GN_ARGS_CMD}
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  RESULT_VARIABLE MAKE_GN_ARGS_RESULT)
  if(NOT MAKE_GN_ARGS_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to write Generate Ninja configuration")
  endif()

  if(BOOTSTRAP_GN)
    _bootstrap_gn()
  endif()

  set(GN_CMD ${GN_EXE} gen ${CHROMIUM_OUTPUT_DIR})
  message(STATUS "Generating Ninja build files")
  execute_process(COMMAND ${GN_CMD}
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  RESULT_VARIABLE GN_RESULT)
  if(NOT GN_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to run Generate Ninja")
  endif()
endfunction()
