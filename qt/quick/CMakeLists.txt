# vim:expandtab:shiftwidth=2:tabstop=2:

# Copyright (C) 2014-2016 Canonical Ltd.

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

qt5_wrap_cpp(MOC_EXTRA
             api/oxideqquickwebcontextdelegateworker_p_p.h)

set(OXIDE_QUICKLIB_SRCS
    api/oxideqquickcookiemanager.cc
    api/oxideqquickglobal.cc
    api/oxideqquicklocationbarcontroller.cc
    api/oxideqquicknavigationhistory.cc
    api/oxideqquicknavigationitem.cc
    api/oxideqquickscriptmessage.cc
    api/oxideqquickscriptmessagehandler.cc
    api/oxideqquickscriptmessagerequest.cc
    api/oxideqquicktouchselectioncontroller.cc
    api/oxideqquickuserscript.cc
    api/oxideqquickwebcontext.cc
    api/oxideqquickwebcontextdelegateworker.cc
    api/oxideqquickwebframe.cc
    api/oxideqquickwebview.cc
    api/oxidequseragentoverriderequest.cc
    contents_view.cc
    legacy_contents_view.cc
    oxide_qquick_accelerated_frame_node.cc
    oxide_qquick_file_picker.cc
    oxide_qquick_image_frame_node.cc
    oxide_qquick_init.cc
    oxide_qquick_software_frame_node.cc
    oxide_qquick_touch_handle_drawable.cc
    qquick_legacy_alert_dialog.cc
    qquick_legacy_auxiliary_ui_factory.cc
    qquick_legacy_before_unload_dialog.cc
    qquick_legacy_confirm_dialog.cc
    qquick_legacy_javascript_dialog.cc
    qquick_legacy_prompt_dialog.cc
    qquick_legacy_web_context_menu.cc
    qquick_legacy_web_popup_menu.cc
    ${MOC_EXTRA})

add_library(${OXIDE_QUICKLIB} SHARED ${OXIDE_QUICKLIB_SRCS})
target_compile_definitions(
    ${OXIDE_QUICKLIB} PRIVATE -DOXIDE_QTQUICK_IMPLEMENTATION)
target_include_directories(
    ${OXIDE_QUICKLIB} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/api/includes)
target_include_directories(
    ${OXIDE_QUICKLIB} PRIVATE
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS})
target_link_libraries(
    ${OXIDE_QUICKLIB}
    PUBLIC  ${OXIDE_LIB} ${Qt5Core_LIBRARIES} ${Qt5Qml_LIBRARIES} ${Qt5Quick_LIBRARIES}
    PRIVATE ${Qt5Gui_LIBRARIES})
if(QTGUI_OPENGL_REQUIRES_GL_LINKING)
  target_link_libraries(${OXIDE_QUICKLIB} PRIVATE ${Qt5Gui_OPENGL_LIBRARIES})
endif()
set_target_properties(
    ${OXIDE_QUICKLIB} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${OXIDE_LIB_OUTPUT_DIR}
    AUTOMOC TRUE
    SOVERSION ${OXIDE_LIB_VERSION})

install(TARGETS ${OXIDE_QUICKLIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/api/
        DESTINATION ${OXIDE_INSTALL_INCLUDEDIR}/OxideQtQuick
        FILES_MATCHING PATTERN *.h
        PATTERN *_p.h EXCLUDE
        PATTERN includes* EXCLUDE)

add_custom_target(
    test-package-qt-quick
    COMMAND ${CMAKE_COMMAND} -DFILES=$<TARGET_FILE:${OXIDE_QUICKLIB}>
    -DTYPE=FILE -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}
    -P ${OXIDE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)

get_library_filename(OXIDE_QUICKLIB_FILENAME ${OXIDE_QUICKLIB} ${OXIDE_LIB_VERSION})
configure_and_install_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/api/OxideQtQuickConfig.cmake.in)
