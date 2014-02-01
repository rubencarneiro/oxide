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
