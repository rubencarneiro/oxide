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

if(DEFINED _Oxide_Options_INCLUDED_)
  return()
endif()
set(_Oxide_Options_INCLUDED_ TRUE)

include(CMakeDependentOption)

option(ENABLE_COMPONENT_BUILD
       "Build all components of the core library as shared objects"
       OFF)
option(ENABLE_TESTS "Enable the tests" OFF)
option(ENABLE_CHROMIUM_TESTS "Enable Chromium tests" ON)
option(ENABLE_PROPRIETARY_CODECS "Enable support for MP3, H.264 and AAC" OFF)

set(_ENABLE_PLUGINS_DEFAULT OFF)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "i.86" OR
   CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  set(_ENABLE_PLUGINS_DEFAULT ON)
endif()
option(ENABLE_PLUGINS
       "Enable support for PPAPI plugins. Only Flash is supported right now"
       ${_ENABLE_PLUGINS_DEFAULT})
unset(_ENABLE_PLUGINS_DEFAULT)

set(_ENABLE_TCMALLOC_DEFAULT OFF)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(_ENABLE_TCMALLOC_DEFAULT ON)
endif()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
  set(_ENABLE_TCMALLOC_DEFAULT OFF)
endif()
option(ENABLE_TCMALLOC "Use TCMalloc in the renderer executable" ${_ENABLE_TCMALLOC_DEFAULT})
unset(_ENABLE_TCMALLOC_DEFAULT)

option(USE_SYSTEM_PROTOBUF "Use the system protobuf" OFF)

set(_IS_LINUX false)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(_IS_LINUX true)
endif()
cmake_dependent_option(ENABLE_HYBRIS "Enable code that uses libhybris"
                       ON "_IS_LINUX" OFF)
unset(_IS_LINUX)

cmake_dependent_option(
    ENABLE_HYBRIS_CAMERA
    "Enable support for the camera compatibility layer in Ubuntu's libhybris"
    OFF "ENABLE_HYBRIS" OFF)

option(BOOTSTRAP_GN "Bootstrap a Generate Ninja binary" OFF)

include(${OXIDE_PLATFORM}/Options)
