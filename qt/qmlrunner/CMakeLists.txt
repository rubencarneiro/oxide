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
include(OxideQtCommon)

set(QMLRUNNER oxideqmlscene)

set(QMLRUNNER_SRCS main.cc)

add_executable(${QMLRUNNER} ${QMLRUNNER_SRCS})
qt5_use_modules(${QMLRUNNER} Core Gui Qml Quick)
target_include_directories(${QMLRUNNER} PRIVATE ${Qt5Core_PRIVATE_INCLUDE_DIRS})
if(OXIDE_ENABLE_COMPOSITING)
  target_compile_definitions(${QMLRUNNER} PRIVATE -DENABLE_COMPOSITING)
  target_include_directories(
      ${QMLRUNNER} PRIVATE
      ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
      ${Qt5Quick_PRIVATE_INCLUDE_DIRS})
endif()
set_target_properties(
    ${QMLRUNNER} PROPERTIES
    SKIP_BUILD_RPATH TRUE
    RUNTIME_OUTPUT_DIRECTORY ${OXIDE_BIN_OUTPUT_DIR})

get_target_property(_QMLRUNNER_LOCATION ${QMLRUNNER} LOCATION)

install(PROGRAMS ${_QMLRUNNER_LOCATION} DESTINATION ${CMAKE_INSTALL_BINDIR})

get_filename_component(_QMLRUNNER_NAME ${_QMLRUNNER_LOCATION} NAME)
add_custom_target(stage-oxideqmlscene
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${_QMLRUNNER_LOCATION} ${OXIDE_STAGE_DIR}/${_QMLRUNNER_NAME})
