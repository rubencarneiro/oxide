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

set(PLUGIN qmloxideplugin)
set(PLUGIN_MODULE_NAME com/canonical/Oxide)

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

qt5_wrap_cpp(MOC_EXTRA
             api/oxideqquickuserscript_p_p.h
             api/oxideqquickwebcontext_p_p.h
             api/oxideqquickwebcontextdelegateworker_p_p.h)

set(PLUGIN_SRCS
    api/oxideqquickcookiemanager.cc
    api/oxideqquickglobals.cc
    api/oxideqquicknavigationhistory.cc
    api/oxideqquickscriptmessage.cc
    api/oxideqquickscriptmessagehandler.cc
    api/oxideqquickscriptmessagerequest.cc
    api/oxideqquickuserscript.cc
    api/oxideqquickwebcontext.cc
    api/oxideqquickwebcontextdelegateworker.cc
    api/oxideqquickwebframe.cc
    api/oxideqquickwebview.cc
    api/oxidequseragentoverriderequest.cc
    oxide_qml_plugin.cc
    oxide_qquick_alert_dialog_delegate.cc
    oxide_qquick_before_unload_dialog_delegate.cc
    oxide_qquick_confirm_dialog_delegate.cc
    oxide_qquick_file_picker_delegate.cc
    oxide_qquick_javascript_dialog_delegate.cc
    oxide_qquick_prompt_dialog_delegate.cc
    oxide_qquick_software_frame_node.cc
    oxide_qquick_web_popup_menu_delegate.cc
    ${MOC_EXTRA})

if(OXIDE_ENABLE_COMPOSITING)
  list(APPEND PLUGIN_SRCS oxide_qquick_accelerated_frame_node.cc)
endif()

add_library(${PLUGIN} MODULE ${PLUGIN_SRCS})
qt5_use_modules(${PLUGIN} Core Gui Qml Quick)
if(OXIDE_ENABLE_COMPOSITING)
  target_compile_definitions(${PLUGIN} PRIVATE -DENABLE_COMPOSITING)
endif()
target_include_directories(
    ${PLUGIN} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS})
target_link_libraries(${PLUGIN} OxideQtCore)
set_target_properties(
    ${PLUGIN} PROPERTIES
    SKIP_BUILD_RPATH TRUE AUTOMOC TRUE
    LIBRARY_OUTPUT_DIRECTORY ${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${PLUGIN_MODULE_NAME})

add_custom_target(
    copy_${PLUGIN}_qmldir ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_CURRENT_SOURCE_DIR}/qmldir
      ${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${QML_PLUGIN_MODULE_NAME})

install(FILES ${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${PLUGIN_MODULE_NAME}/qmldir
        DESTINATION ${QT_INSTALL_QML}/${PLUGIN_MODULE_NAME})
install(TARGETS ${PLUGIN} DESTINATION ${QT_INSTALL_QML}/${PLUGIN_MODULE_NAME})

get_target_property(_PLUGIN_LOCATION ${PLUGIN} LOCATION)
oxide_add_package_files(${_PLUGIN_LOCATION}
                        ${OXIDE_QMLPLUGIN_OUTPUT_DIR}/${PLUGIN_MODULE_NAME}/qmldir)
