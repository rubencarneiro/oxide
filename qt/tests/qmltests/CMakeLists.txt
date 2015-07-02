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

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5QuickTest REQUIRED)
find_package(Qt5Test REQUIRED)

set(QML_PLUGIN qmloxidetestingplugin)
set(QML_PLUGIN_MODULE_NAME com/canonical/Oxide/Testing)

set(QML_PLUGIN_SRCS oxide_qml_testing_plugin.cc)

add_library(${QML_PLUGIN} MODULE ${QML_PLUGIN_SRCS})
target_link_libraries(${QML_PLUGIN}
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Test
    ${OXIDE_QUICKLIB})
set_target_properties(
    ${QML_PLUGIN} PROPERTIES
    AUTOMOC TRUE
    LIBRARY_OUTPUT_DIRECTORY ${QMLPLUGIN_OUTPUT_DIR}/${QML_PLUGIN_MODULE_NAME})

foreach(f
    qmldir
    ScriptMessageTestHandler.qml
    ScriptMessageTestUtils.js
    ScriptMessageTestUtilsUserScript.js
    TestUtils.js
    TestUtilsSlave.js
    TestWebContext.qml
    TestWebView.qml)
  add_custom_target(
      copy_${QML_PLUGIN}_${f} ALL
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/${f}
        ${QMLPLUGIN_OUTPUT_DIR}/${QML_PLUGIN_MODULE_NAME}
      DEPENDS ${QML_PLUGIN})
endforeach()

add_custom_target(
    mkdir_${QML_PLUGIN}.hack ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory
      ${QMLPLUGIN_OUTPUT_DIR}/${QML_PLUGIN_MODULE_NAME}/hack)
add_custom_target(
    copy_${QML_PLUGIN}_qmldir.hack ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_CURRENT_SOURCE_DIR}/qmldir.hack
      ${QMLPLUGIN_OUTPUT_DIR}/${QML_PLUGIN_MODULE_NAME}/hack/qmldir
    DEPENDS mkdir_${QML_PLUGIN}.hack)
add_custom_target(
    copy_${QML_PLUGIN}_plugin.hack ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:${QML_PLUGIN}>
      ${QMLPLUGIN_OUTPUT_DIR}/${QML_PLUGIN_MODULE_NAME}/hack/
    DEPENDS ${QML_PLUGIN} mkdir_${QML_PLUGIN}.hack)

set(QML_TEST qmltest)
set(QML_TEST_SRCS main.cc)

add_executable(${QML_TEST} ${QML_TEST_SRCS})
target_link_libraries(${QML_TEST}
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickTest
    Qt5::Test
    ${OXIDE_CORELIB_NAME})
target_include_directories(
    ${QML_TEST} PRIVATE
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
    ${Qt5QuickTest_PRIVATE_INCLUDE_DIRS})
set_target_properties(
    ${QML_TEST} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_OUTPUT_DIR}
    AUTOMOC TRUE)

function(add_qml_test)
  cmake_parse_arguments(_ARGS "SSL_TESTS" "NAME;TEST_DIR;SERVER_DIR" "" ${ARGN})
  if(NOT _ARGS_NAME)
    message(FATAL_ERROR "You must specify NAME when calling add_qml_test")
  endif()
  if(NOT _ARGS_TEST_DIR)
    message(FATAL_ERROR "You must specify DATA_DIR when calling add_qml_test")
  endif()
  if(NOT _ARGS_SERVER_DIR)
    set(_ARGS_SERVER_DIR ${_ARGS_TEST_DIR})
  endif()

  set(TEST_COMMAND
      ${BIN_OUTPUT_DIR}/run_qmlapp.sh
      ${PYTHON} ${CMAKE_SOURCE_DIR}/qt/tests/runtests.py
      --server-dir ${_ARGS_SERVER_DIR} --server-config 8080)

  if(_ARGS_SSL_TESTS)
    list(APPEND TEST_COMMAND
         --server-config "4443:${CMAKE_SOURCE_DIR}/qt/tests/ssldata/OxideTest"
         --server-config "4444:${CMAKE_SOURCE_DIR}/qt/tests/ssldata/OxideTestExpired"
         --server-config "4445:${CMAKE_SOURCE_DIR}/qt/tests/ssldata/OxideTestSelfSigned"
         --server-config "4446:${CMAKE_SOURCE_DIR}/qt/tests/ssldata/OxideTestBadIdentity")
  endif()

  list(APPEND TEST_COMMAND --
       $<TARGET_FILE:${QML_TEST}> -name ${_ARGS_NAME}
       -input ${_ARGS_TEST_DIR}
       -add-library-path ${QTPLUGIN_OUTPUT_DIR})
  if (_ARGS_SSL_TESTS)
    list(APPEND TEST_COMMAND -nss-db-path ${CMAKE_SOURCE_DIR}/qt/tests/ssldata/nss)
  endif()

  add_test(NAME ${_ARGS_NAME} COMMAND ${TEST_COMMAND})
endfunction()

add_qml_test(NAME qml-api-test TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/api)
add_qml_test(NAME qml-core-test TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/core)
add_qml_test(NAME qml-crash-test TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/crash)
add_qml_test(NAME qml-ssl-test TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ssl SSL_TESTS)
