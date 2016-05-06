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

if(DEFINED _AutoGenerateHelper_INCLUDED_)
  return()
endif()
set(_AutoGenerateHelper_INCLUDED_ TRUE)

include(CMakeParseArguments)
include(OxideCommonProperties)

function(auto_generate_file)
  cmake_parse_arguments(_ARGS "" "INPUT;OUTPUT" "VARS" "${ARGN}")

  if(_ARGS_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords ${_ARGS_UNPARSED_ARGUMENTS}")
  endif()

  set(_CMD ${CMAKE_COMMAND})

  foreach(var ${_ARGS_VARS})
    if(NOT DEFINED ${var})
      message(FATAL_ERROR "Variable ${var} does not exist")
    endif()
    if(${var} STREQUAL "__INPUT" OR ${var} STREQUAL "__OUTPUT")
      message(FATAL_ERROR "Cannot use reserved variables")
    endif()
    list(APPEND _CMD "-D${var}=${${var}}")
  endforeach()

  if(NOT DEFINED _ARGS_OUTPUT)
    message(FATAL_ERROR "Must specify an output file")
  endif()

  if(NOT DEFINED _ARGS_INPUT)
    file(RELATIVE_PATH _FILE ${CMAKE_CURRENT_BINARY_DIR} ${_ARGS_OUTPUT})
    set(_ARGS_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/${_FILE}.in)
  endif()

  set(_HELPER "${OXIDE_SOURCE_DIR}/build/scripts/preprocess-helper.cmake")

  list(APPEND _CMD "-D__INPUT=${_ARGS_INPUT}" "-D__OUTPUT=${_ARGS_OUTPUT}")
  list(APPEND _CMD "-P" "${_HELPER}")

  string(REPLACE "_" "__" _TARGET_NAME_SUFFIX ${_ARGS_OUTPUT})
  string(REPLACE "/" "_" _TARGET_NAME_SUFFIX ${_TARGET_NAME_SUFFIX})

  add_custom_command(OUTPUT ${_ARGS_OUTPUT}
                     COMMAND ${_CMD}
                     DEPENDS ${_ARGS_INPUT} ${_HELPER}
                     COMMENT "Generating file ${_ARGS_OUTPUT}")
  add_custom_target(auto-generate-file-${_TARGET_NAME_SUFFIX}
                    ALL DEPENDS ${_ARGS_OUTPUT})
endfunction()
