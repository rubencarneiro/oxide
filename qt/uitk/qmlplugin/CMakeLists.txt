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

set(OXIDE_UITK_QMLPLUGIN qmloxideuitkplugin)
set(OXIDE_UITK_QMLPLUGIN_MODULE_NAME Oxide/Ubuntu)

set(OXIDE_UITK_QMLPLUGIN_SRCS oxide_ubuntu_qml_plugin.cc)

# CMake doesn't seem to provide a way to get at QT_INSTALL_QML
if(NOT CMAKE_CROSSCOMPILING)
  execute_process(COMMAND ${QMAKE_EXECUTABLE} -qt5 -query QT_INSTALL_QML
                  RESULT_VARIABLE _RESULT
                  OUTPUT_VARIABLE QT_INSTALL_QML
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT _RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to determine QT_INSTALL_QML from qmake")
  endif()
else()
  set(QT_INSTALL_QML "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml")
endif()

add_library(${OXIDE_UITK_QMLPLUGIN} MODULE ${OXIDE_UITK_QMLPLUGIN_SRCS})
target_link_libraries(${OXIDE_UITK_QMLPLUGIN}
                      ${Qt5Qml_LIBRARIES}
                      ${OXIDE_UITKLIB}
                      ${OXIDE_QUICKLIB})
set_target_properties(
    ${OXIDE_UITK_QMLPLUGIN} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${OXIDE_UITK_QMLPLUGIN_MODULE_NAME}
    AUTOMOC TRUE)

add_custom_target(
    copy_${OXIDE_UITK_QMLPLUGIN}_qmldir ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_CURRENT_SOURCE_DIR}/qmldir
      ${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${OXIDE_UITK_QMLPLUGIN_MODULE_NAME})

install(FILES ${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${OXIDE_UITK_QMLPLUGIN_MODULE_NAME}/qmldir
        DESTINATION ${QT_INSTALL_QML}/${OXIDE_UITK_QMLPLUGIN_MODULE_NAME})
install(TARGETS ${OXIDE_UITK_QMLPLUGIN}
        DESTINATION ${QT_INSTALL_QML}/${OXIDE_UITK_QMLPLUGIN_MODULE_NAME})

add_custom_target(
    test-package-uitk-qmlplugin
    COMMAND ${CMAKE_COMMAND}
    "-DFILES=\"$<TARGET_FILE:${OXIDE_UITK_QMLPLUGIN}>;${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${OXIDE_UITK_QMLPLUGIN_MODULE_NAME}/qmldir\""
    -DTYPE=FILE
    -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}/${OXIDE_QT_PACKAGE_QMLPLUGIN_DIR}/${OXIDE_UITK_QMLPLUGIN_MODULE_NAME}
    -P ${OXIDE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)
