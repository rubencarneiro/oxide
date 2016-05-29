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

function(add_chromium_build_all_target name)
  find_program(NINJA ninja)
  if(NINJA STREQUAL "NINJA-NOTFOUND")
    message(FATAL_ERROR "Could not find ninja, which is required for building Oxide")
  endif()
  set(NINJA_CMD ${NINJA} -C ${CHROMIUM_OUTPUT_DIR})
  if(CMAKE_VERBOSE_MAKEFILE)
    list(APPEND NINJA_CMD -v)
  endif()
  list(APPEND NINJA_CMD oxide_all)
  add_custom_target(
      ${name} ALL COMMAND ${NINJA_CMD}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      COMMENT "Running the Chromium build all target")
endfunction()

function(run_generate_ninja)
  if(NOT DEFINED OXIDE_LIB OR
     NOT DEFINED OXIDE_LIB_VERSION OR
     NOT DEFINED OXIDE_RENDERER)
    message(FATAL_ERROR "One or more parameters are missing from the build")
  endif()

  find_program(GN_OXIDE gn.oxide)
  if(GN_OXIDE STREQUAL "GN_OXIDE-NOTFOUND")
    message(FATAL_ERROR
            "Can't find gn binary. Did you check out the source following "
            "the instructions at https://wiki.ubuntu.com/Oxide/GetTheCode?")
  endif()

  set(MAKE_GN_ARGS_CMD
      ${PYTHON} ${OXIDE_SOURCE_DIR}/build/scripts/make_gn_args.py)

  if(ENABLE_COMPONENT_BUILD)
    list(APPEND MAKE_GN_ARGS_CMD --component-build)
  endif()
  list(APPEND MAKE_GN_ARGS_CMD --libexec-subdir ${OXIDE_PLATFORM_FULLNAME})

  get_library_extension(LIB_EXTENSION ${OXIDE_LIB_VERSION})
  list(APPEND MAKE_GN_ARGS_CMD --lib-extension ${LIB_EXTENSION})

  list(APPEND MAKE_GN_ARGS_CMD --lib-name ${OXIDE_LIB})
  list(APPEND MAKE_GN_ARGS_CMD --output-dir ${CHROMIUM_OUTPUT_DIR})
  list(APPEND MAKE_GN_ARGS_CMD --platform ${OXIDE_PLATFORM})
  list(APPEND MAKE_GN_ARGS_CMD --renderer-name ${OXIDE_RENDERER})

  if(CMAKE_CROSSCOMPILING)
    if(NOT CHROMIUM_TARGET_ARCH)
      message(FATAL_ERROR "Need to set CHROMIUM_TARGET_ARCH when cross compiling")
    endif()
    list(APPEND MAKE_GN_ARGS_CMD -Dtarget_arch=${CHROMIUM_TARGET_ARCH})
  endif()

  if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND MAKE_GN_ARGS_CMD -Dis_debug=true)
  else()
    list(APPEND MAKE_GN_ARGS_CMD -Dis_debug=false)
  endif()

  if(NOT DEFINED CMAKE_BUILD_TYPE OR NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND MAKE_GN_ARGS_CMD --enable-debug-symbols)
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
  else()
    list(APPEND MAKE_GN_ARGS_CMD -Denable_tests=false)
  endif()

  if(CMAKE_CROSSCOMPILING)
    message(FATAL_ERROR "Add support for cross-compiling")
  endif()

  message(STATUS "Writing Generate Ninja configuration")
  execute_process(COMMAND ${MAKE_GN_ARGS_CMD}
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  RESULT_VARIABLE MAKE_GN_ARGS_RESULT)
  if(NOT MAKE_GN_ARGS_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to write Generate Ninja configuration")
  endif()

  set(GN_CMD ${GN_OXIDE} gen ${CHROMIUM_OUTPUT_DIR})
  message(STATUS "Running Generate Ninja")
  execute_process(COMMAND ${GN_CMD}
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  RESULT_VARIABLE GN_RESULT)
  if(NOT GN_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to run Generate Ninja")
  endif()
endfunction()
