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

# CMake doesn't seem to provide a way to get at QT_INSTALL_QML
if(NOT CMAKE_CROSSCOMPILING)
  find_program(QMAKE_EXECUTABLE qmake)
  if(QMAKE_EXECUTABLE STREQUAL "QMAKE_EXECUTABLE-NOTFOUND")
    message(FATAL_ERROR "qmake not found")
  endif()
  execute_process(COMMAND ${QMAKE_EXECUTABLE} -query QT_INSTALL_QML
                  RESULT_VARIABLE _RESULT
                  OUTPUT_VARIABLE QT_INSTALL_QML
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT _RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to determine QT_INSTALL_QML from qmake")
  endif()
else()
  # qmake isn't multi-arch aware as it installs arch-specific mkspec files
  # in to /usr/share, so we can't use it here (we'd need a qmake binary
  # for the host arch using data for the target arch)
  set(QT_INSTALL_QML "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml")
endif()

set(OXIDE_QMLPLUGIN_SRCS oxide_qml_plugin.cc)

if(GENERATE_QMLTYPES)
  execute_process(
      COMMAND  qmlplugindump com.canonical.Oxide ${OXIDE_VERSION_MAJOR}.${OXIDE_VERSION_MINOR} .
      OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/oxide.qmltypes
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
endif()

add_library(${OXIDE_QMLPLUGIN} MODULE ${OXIDE_QMLPLUGIN_SRCS})

qt5_use_modules(${OXIDE_QMLPLUGIN} Core Qml Quick)
target_link_libraries(
    ${OXIDE_QMLPLUGIN}
    ${OXIDE_CORELIB_NAME}
    ${OXIDE_QUICKLIB})
set_target_properties(
    ${OXIDE_QMLPLUGIN} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${QMLPLUGIN_OUTPUT_DIR}/${OXIDE_QMLPLUGIN_MODULE_NAME}
    AUTOMOC TRUE)

add_custom_target(
    copy_${OXIDE_QMLPLUGIN}_qmldir ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_CURRENT_SOURCE_DIR}/qmldir
      ${QMLPLUGIN_OUTPUT_DIR}/${OXIDE_QMLPLUGIN_MODULE_NAME})

install(FILES ${QMLPLUGIN_OUTPUT_DIR}/${OXIDE_QMLPLUGIN_MODULE_NAME}/qmldir
        DESTINATION ${QT_INSTALL_QML}/${OXIDE_QMLPLUGIN_MODULE_NAME})
install(TARGETS ${OXIDE_QMLPLUGIN} DESTINATION ${QT_INSTALL_QML}/${OXIDE_QMLPLUGIN_MODULE_NAME})
