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

set(OXIDE_QMLRUNNER_SRCS main.cc)

add_executable(${OXIDE_QMLRUNNER} ${OXIDE_QMLRUNNER_SRCS})
qt5_use_modules(${OXIDE_QMLRUNNER} Core Gui Qml Quick)
target_include_directories(${OXIDE_QMLRUNNER} PRIVATE ${Qt5Core_PRIVATE_INCLUDE_DIRS})
if(OXIDE_ENABLE_COMPOSITING)
  target_compile_definitions(${OXIDE_QMLRUNNER} PRIVATE -DENABLE_COMPOSITING)
  target_include_directories(
      ${OXIDE_QMLRUNNER} PRIVATE
      ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
      ${Qt5Quick_PRIVATE_INCLUDE_DIRS})
endif()
set_target_properties(
    ${OXIDE_QMLRUNNER} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR})

install(TARGETS ${OXIDE_QMLRUNNER} DESTINATION ${CMAKE_INSTALL_BINDIR})
