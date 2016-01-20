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

if(CMAKE_CROSSCOMPILING)
  # QT_MOC_EXECUTABLE is set by Qt5CoreConfigExtras, but it sets it to
  # the target executable rather than the host executable, which is no
  # use for cross-compiling. For cross-compiling, we have a guess and
  # override it ourselves
  if(NOT TARGET Qt5::moc)
    find_program(
        QT_MOC_EXECUTABLE moc
        PATHS /usr/lib/qt5/bin /usr/lib/${OXIDE_LIBRARY_HOST_ARCHITECTURE}/qt5/bin
        NO_DEFAULT_PATH)
    if(QT_MOC_EXECUTABLE STREQUAL "QT_MOC_EXECUTABLE-NOTFOUND")
      message(FATAL_ERROR "Can't find a moc executable for the host arch")
    endif()
    add_executable(Qt5::moc IMPORTED)
    set_target_properties(Qt5::moc PROPERTIES
        IMPORTED_LOCATION "${QT_MOC_EXECUTABLE}")
  endif()
else()
  # This should be enough to initialize QT_MOC_EXECUTABLE
  find_package(Qt5Core REQUIRED)
  if(NOT DEFINED QT_MOC_EXECUTABLE)
    message(FATAL_ERROR "Qt5Core didn't define QT_MOC_EXECUTABLE")
  endif()
endif()

set(OXIDE_PROJECT_NAME oxide-qt)
set(OXIDE_LIB OxideQtCore)
set(OXIDE_LIB_VERSION 0)
set(OXIDE_RENDERER oxide-renderer)

list(APPEND OXIDE_GYP_EXTRA_ARGS -Dqt_moc_executable=${QT_MOC_EXECUTABLE})
