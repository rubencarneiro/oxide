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

set(OXIDE_QMLPLUGIN qmloxideplugin)
set(OXIDE_QMLPLUGIN_MODULE_NAME com/canonical/Oxide)

find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

set(OXIDE_QMLPLUGIN_SRCS oxide_qml_plugin.cc)

if(${Qt5Qml_VERSION_STRING} VERSION_LESS "5.5.0")
  list(APPEND OXIDE_QMLPLUGIN_SRCS
       oxide_qml_download_request.cc
       oxide_qml_load_event.cc
       oxide_qml_ssl_certificate.cc
       oxide_qml_value_type_provider.cc)
  set(LEGACY_QMLVALUE_TYPES TRUE)
endif()

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

add_library(${OXIDE_QMLPLUGIN} MODULE ${OXIDE_QMLPLUGIN_SRCS})
target_link_libraries(
    ${OXIDE_QMLPLUGIN}
    ${Qt5Core_LIBRARIES}
    ${Qt5Qml_LIBRARIES}
    ${Qt5Quick_LIBRARIES}
    ${OXIDE_LIB}
    ${OXIDE_QUICKLIB})
set_target_properties(
    ${OXIDE_QMLPLUGIN} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${OXIDE_QMLPLUGIN_MODULE_NAME}
    AUTOMOC TRUE)
target_compile_options(${OXIDE_QMLPLUGIN} PRIVATE -fno-rtti)
target_include_directories(
    ${OXIDE_QMLPLUGIN} PRIVATE
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Qml_PRIVATE_INCLUDE_DIRS})
if(DEFINED LEGACY_QMLVALUE_TYPES)
  target_compile_definitions(
      ${OXIDE_QMLPLUGIN} PRIVATE -DLEGACY_QMLVALUE_TYPES)
endif()

add_custom_target(
    copy_${OXIDE_QMLPLUGIN}_qmldir ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_CURRENT_SOURCE_DIR}/qmldir
      ${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${OXIDE_QMLPLUGIN_MODULE_NAME})

install(FILES ${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${OXIDE_QMLPLUGIN_MODULE_NAME}/qmldir
        DESTINATION ${QT_INSTALL_QML}/${OXIDE_QMLPLUGIN_MODULE_NAME})
install(TARGETS ${OXIDE_QMLPLUGIN}
        DESTINATION ${QT_INSTALL_QML}/${OXIDE_QMLPLUGIN_MODULE_NAME})

add_custom_target(
    test-package-qt-qmlplugin
    COMMAND ${CMAKE_COMMAND}
    "-DFILES=\"$<TARGET_FILE:${OXIDE_QMLPLUGIN}>;${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${OXIDE_QMLPLUGIN_MODULE_NAME}/qmldir\""
    -DTYPE=FILE
    -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}/${OXIDE_QT_PACKAGE_QMLPLUGIN_DIR}/${OXIDE_QMLPLUGIN_MODULE_NAME}
    -P ${CMAKE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)

# Only generate typeinfo if not cross compiling
if(NOT CMAKE_CROSSCOMPILING)
  add_custom_target(generate_oxide_qmltypes ALL
    COMMAND env "LD_LIBRARY_PATH=${CHROMIUM_OUTPUT_DIR}/Release/lib:${OXIDE_LIB_OUTPUT_DIR}"
            env "QML2_IMPORT_PATH=${OXIDE_QMLPLUGIN_OUTPUT_DIR}"
            qmlplugindump -v -noinstantiate com.canonical.Oxide ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR} ${OXIDE_QMLPLUGIN_MODULE_NAME} > ${CMAKE_CURRENT_BINARY_DIR}/oxide.qmltypes
            && cp ${CMAKE_CURRENT_BINARY_DIR}/oxide.qmltypes ${CMAKE_CURRENT_SOURCE_DIR}/
            || true
    WORKING_DIRECTORY ${OXIDE_QMLPLUGIN_OUTPUT_DIR}
  )
  add_dependencies(generate_oxide_qmltypes ${OXIDE_QMLPLUGIN})

endif()

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/oxide.qmltypes
        DESTINATION ${QT_INSTALL_QML}/${OXIDE_QMLPLUGIN_MODULE_NAME})
