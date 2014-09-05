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

include(CMakeParseArguments)

set(QML_PLUGIN qmloxidetestingplugin)
set(QML_PLUGIN_MODULE_NAME com/canonical/Oxide/Testing)

set(QML_PLUGIN_SRCS oxide_qml_testing_plugin.cc)

add_library(${QML_PLUGIN} MODULE ${QML_PLUGIN_SRCS})
qt5_use_modules(${QML_PLUGIN} Core Qml)
target_include_directories(
    ${QML_PLUGIN} PRIVATE
    ${CMAKE_SOURCE_DIR})
set_target_properties(
    ${QML_PLUGIN} PROPERTIES
    SKIP_BUILD_RPATH TRUE AUTOMOC TRUE
    LIBRARY_OUTPUT_DIRECTORY ${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${QML_PLUGIN_MODULE_NAME})

foreach(f qmldir TestUtils.js TestUtilsSlave.js TestWebContext.qml TestWebView.qml)
  add_custom_target(
      copy_${QML_PLUGIN}_${f} ALL
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/${f}
        ${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${QML_PLUGIN_MODULE_NAME})
endforeach()

set(QML_TEST qmltest)
set(QML_TEST_SRCS main.cc)

add_executable(${QML_TEST} ${QML_TEST_SRCS})
qt5_use_modules(${QML_TEST} Core Gui Qml Quick QuickTest)
target_include_directories(
    ${QML_TEST} PRIVATE
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS})
if(OXIDE_ENABLE_COMPOSITING)
  target_compile_definitions(${QML_TEST} PRIVATE -DENABLE_COMPOSITING)
endif()
set_target_properties(
    ${QML_TEST} PROPERTIES
    SKIP_BUILD_RPATH TRUE
    RUNTIME_OUTPUT_DIRECTORY ${OXIDE_BIN_OUTPUT_DIR})

function(add_qml_test)
  cmake_parse_arguments(_ARGS "USE_DATA_DIR" "NAME;TEST_DIR;SERVER_DIR" "" ${ARGN})
  if(NOT _ARGS_NAME)
    message(FATAL_ERROR "You must specify NAME when calling add_qml_test")
  endif()
  if(NOT _ARGS_TEST_DIR)
    message(FATAL_ERROR "You must specify DATA_DIR when calling add_qml_test")
  endif()
  if(NOT _ARGS_SERVER_DIR)
    set(_ARGS_SERVER_DIR ${_ARGS_TEST_DIR})
  endif()

  get_target_property(QMLTEST_BINARY ${QML_TEST} LOCATION)

  set(TEST_COMMAND
      ${CMAKE_BINARY_DIR}/out/run_qmlapp.sh
      ${PYTHON} ${CMAKE_SOURCE_DIR}/qt/tests/runtests.py
      -s ${_ARGS_SERVER_DIR} -p 8080)
  if(_ARGS_USE_DATA_DIR)
    list(APPEND TEST_COMMAND -t)
  endif()
  list(APPEND TEST_COMMAND
       --
       ${QMLTEST_BINARY}
       -name ${_ARGS_NAME}
       -input ${_ARGS_TEST_DIR}
       --add-library-path ${CMAKE_CURRENT_BINARY_DIR}/mock)

  add_test(NAME ${_ARGS_NAME} COMMAND ${TEST_COMMAND})
endfunction()

add_qml_test(NAME qml-api-test-noprofile TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/api)
add_qml_test(NAME qml-api-test TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/api USE_DATA_DIR)
add_qml_test(NAME qml-core-test TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/core)
add_qml_test(NAME qml-crash-test TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/crash)
