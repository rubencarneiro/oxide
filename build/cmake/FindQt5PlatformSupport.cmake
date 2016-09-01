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

include(FindPkgConfig)

# Try using pkg-config first
pkg_check_modules(Qt5PlatformSupport QUIET Qt5PlatformSupport)

if(NOT DEFINED ${Qt5PlatformSupport_FOUND})
  set(Qt5PlatformSupport_FOUND 1 CACHE INTERNAL "")
  find_package(Qt5Core REQUIRED)
  find_package(Qt5Gui REQUIRED)

  find_file(_HEADER_PATH qt5/QtPlatformSupport/QtPlatformSupport)
  if(_HEADER_PATH STREQUAL "_HEADER_PATH-NOTFOUND")
    message(FATAL_ERROR "Can't find QtPlatformSupport header")
  endif()
  get_filename_component(_HEADER_DIR ${_HEADER_PATH} DIRECTORY)
  get_filename_component(Qt5PlatformSupport_INCLUDEDIR ${_HEADER_DIR} DIRECTORY)
  set(Qt5PlatformSupport_INCLUDEDIR ${Qt5PlatformSupport_INCLUDEDIR} CACHE INTERNAL "")

  set(Qt5PlatformSupport_INCLUDE_DIRS "${_HEADER_DIR};${Qt5PlatformSupport_INCLUDEDIR}")
  list(APPEND Qt5PlatformSupport_INCLUDE_DIRS ${Qt5Gui_INCLUDE_DIRS})
  list(APPEND Qt5PlatformSupport_INCLUDE_DIRS ${Qt5Core_INCLUDE_DIRS})
  set(Qt5PlatformSupport_INCLUDE_DIRS ${Qt5PlatformSupport_INCLUDE_DIRS} CACHE INTERNAL "")
  unset(_HEADER_PATH CACHE)

  find_library(_LIB_PATH Qt5PlatformSupport)
  if(_LIB_PATH STREQUAL "_LIB_PATH-NOTFOUND")
    message(FATAL_ERROR "Can't find Qt5PlatformSupport library")
  endif()
  set(Qt5PlatformSupport_LIBRARIES "Qt5PlatformSupport;Qt5Gui;Qt5Core" CACHE INTERNAL "")
  unset(_LIB_PATH CACHE)

  set(Qt5PlatformSupport_VERSION ${Qt5Gui_VERSION} CACHE INTERNAL "")
endif()

set(Qt5PlatformSupport_PRIVATE_INCLUDE_DIRS
    ${Qt5PlatformSupport_INCLUDEDIR}/QtPlatformSupport/${Qt5PlatformSupport_VERSION}
    ${Qt5PlatformSupport_INCLUDEDIR}/QtPlatformSupport/${Qt5PlatformSupport_VERSION}/QtPlatformSupport)
