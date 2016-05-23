# vim:expandtab:shiftwidth=2:tabstop=2:

# Copyright (C) 2014-2015 Canonical Ltd.

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

set(OXIDE_QMLRUNNER oxideqmlscene)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

set(OXIDE_QMLRUNNER_SRCS main.cc)

add_executable(${OXIDE_QMLRUNNER} ${OXIDE_QMLRUNNER_SRCS})
target_link_libraries(
    ${OXIDE_QMLRUNNER} 
    ${Qt5Core_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${Qt5Qml_LIBRARIES}
    ${Qt5Quick_LIBRARIES})
target_include_directories(
    ${OXIDE_QMLRUNNER} PRIVATE
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS})
set_target_properties(
    ${OXIDE_QMLRUNNER} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OXIDE_BIN_OUTPUT_DIR})

install(TARGETS ${OXIDE_QMLRUNNER}
        DESTINATION ${CMAKE_INSTALL_BINDIR})
