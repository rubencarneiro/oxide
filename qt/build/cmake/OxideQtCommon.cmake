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

if(__INCLUDED_OXIDE_QT_COMMON_CMAKE)
  return()
endif()
set(__INCLUDED_OXIDE_QT_COMMON_CMAKE TRUE)

include(CheckCXXSymbolExists)
include(CMakeExpandImportedTargets)
include(GNUInstallDirs)
include(OxideCommon)
include(${CMAKE_SOURCE_DIR}/qt/config.cmake)

set(CMAKE_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_LIBDIR}/${OXIDE_SUBPROCESS_DIR})
set(OXIDE_CORELIB_UNVERSIONED
    ${CMAKE_SHARED_LIBRARY_PREFIX}${OXIDE_CORELIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
set(OXIDE_CORELIB_VERSIONED ${OXIDE_CORELIB_UNVERSIONED}.${OXIDE_CORELIB_SO_VERSION})
set(OXIDE_QMLPLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/imports)
set(OXIDE_BIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/bin)
set(OXIDE_STAGE_DIR ${CMAKE_BINARY_DIR}/out/oxide)

if(NOT TARGET ${OXIDE_CORELIB_NAME})
  # Allow objects linked by cmake to declare a run-time dependency on the
  # main library, built with gyp
  add_library(${OXIDE_CORELIB_NAME} SHARED IMPORTED)
  set_target_properties(
      ${OXIDE_CORELIB_NAME}
      PROPERTIES IMPORTED_LOCATION ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB_UNVERSIONED})
endif()

find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Quick)

set(CMAKE_REQUIRED_INCLUDES
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Quick_INCLUDE_DIRS}
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS})
set(CMAKE_REQUIRED_DEFINITIONS
    ${Qt5Core_DEFINITIONS}
    ${Qt5Gui_DEFINITIONS}
    ${Qt5Quick_DEFINITIONS})
set(CMAKE_REQUIRED_FLAGS -fPIC)
cmake_expand_imported_targets(
    CMAKE_REQUIRED_LIBRARIES
    LIBRARIES ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Quick_LIBRARIES})
check_cxx_symbol_exists(
    QSGContext::setSharedOpenGLContext
    QtQuick/private/qsgcontext_p.h
    OXIDE_ENABLE_COMPOSITING)
