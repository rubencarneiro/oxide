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

include(AutoGenerateHelper)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5QuickTest REQUIRED)
find_package(Qt5Test REQUIRED)

set(QML_SUPPORT_MODULE_NAME Oxide/testsupport)
set(QML_SUPPORT_MODULE_DIR ${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${QML_SUPPORT_MODULE_NAME})

add_custom_command(
    OUTPUT ${QML_SUPPORT_MODULE_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${QML_SUPPORT_MODULE_DIR})
add_custom_target(
    make_testsupport_dir ALL
    DEPENDS ${QML_SUPPORT_MODULE_DIR})
    
foreach(f
    qmldir
    ScriptMessageTestHandler.qml
    ScriptMessageTestUtils.js
    ScriptMessageTestUtilsUserScript.js
    TestUtils.js
    TestUtilsSlave.js
    TestWebContext.qml
    TestWebView.qml)
  add_custom_command(
      OUTPUT ${QML_SUPPORT_MODULE_DIR}/${f}
      COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/${f} ${QML_SUPPORT_MODULE_DIR}/${f}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${f} ${QML_SUPPORT_MODULE_DIR})
  add_custom_target(
      copy_testsupport_file_${f} ALL
      DEPENDS ${QML_SUPPORT_MODULE_DIR}/${f})
endforeach()

set(QML_TEST_SRCS
    main.cc
    qml_test_support.cc
    quick_test_compat.cc
    test_nam_factory.cc)

function(add_qml_test)
  cmake_parse_arguments(_ARGS "" "EXEC;NAME;TEST_DIR" "" "${ARGN}")

  if(NOT DEFINED _ARGS_EXEC)
    message(FATAL_ERROR "Missing option EXEC")
  endif()
  if(NOT DEFINED _ARGS_NAME)
    message(FATAL_ERROR "Missing option NAME")
  endif()

  if(NOT DEFINED _ARGS_TEST_DIR)
    set(_ARGS_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  add_executable(${_ARGS_EXEC} ${QML_TEST_SRCS})
  target_compile_definitions(
      ${_ARGS_EXEC} PRIVATE
      -DQML_TEST_NAME="${_ARGS_NAME}"
      -DQML_TEST_PATH="${_ARGS_TEST_DIR}")
  target_link_libraries(
      ${_ARGS_EXEC}
      Qt5::Core
      Qt5::Gui
      Qt5::Qml
      Qt5::Quick
      Qt5::QuickTest
      Qt5::Test
      ${OXIDE_LIB}
      ${OXIDE_QUICKLIB})
  target_include_directories(
      ${_ARGS_EXEC} PRIVATE
      ${Qt5Core_PRIVATE_INCLUDE_DIRS}
      ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
      ${Qt5Quick_PRIVATE_INCLUDE_DIRS}
      ${Qt5QuickTest_PRIVATE_INCLUDE_DIRS})
  set_target_properties(${_ARGS_EXEC} PROPERTIES AUTOMOC TRUE)

  set(_CONFIG ${OXIDE_OUTPUT_DIR}/test_configs/${_ARGS_NAME}.conf)

  set(QML_TEST_EXEC $<TARGET_FILE:${_ARGS_EXEC}>)
  set(QT_PLUGIN_PATH ${OXIDE_QTPLUGIN_OUTPUT_DIR})
  set(SERVER_DIR ${_ARGS_TEST_DIR})

  auto_generate_file(OUTPUT ${_CONFIG}
                     INPUT ${CMAKE_CURRENT_SOURCE_DIR}/qml-test.conf.in
                     VARS QML_TEST_EXEC QT_PLUGIN_PATH SERVER_DIR)

  set(_COMMAND
      ${OXIDE_BIN_OUTPUT_DIR}/run_qmlapp.sh
      ${PYTHON} ${CMAKE_SOURCE_DIR}/qt/tests/runtests.py
      --config ${_CONFIG})
  add_test(NAME ${_ARGS_NAME} COMMAND ${_COMMAND})
endfunction()

add_qml_test(EXEC qmltest_api NAME qml-api-test TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/api)
add_qml_test(EXEC qmltest_core NAME qml-core-test TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/core)
add_qml_test(EXEC qmltest_crash NAME qml-crash-test TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/crash)
add_qml_test(EXEC qmltest_ssl NAME qml-ssl-test TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ssl)
