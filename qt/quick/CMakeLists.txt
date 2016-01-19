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

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)

qt5_wrap_cpp(MOC_EXTRA
             api/oxideqquickwebcontextdelegateworker_p_p.h)

set(OXIDE_QUICKLIB_SRCS
    api/oxideqquickcookiemanager.cc
    api/oxideqquickglobal.cc
    api/oxideqquicklocationbarcontroller.cc
    api/oxideqquicknavigationhistory.cc
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
    oxide_qquick_accelerated_frame_node.cc
    oxide_qquick_alert_dialog.cc
    oxide_qquick_before_unload_dialog.cc
    oxide_qquick_confirm_dialog.cc
    oxide_qquick_file_picker.cc
    oxide_qquick_image_frame_node.cc
    oxide_qquick_init.cc
    oxide_qquick_javascript_dialog.cc
    oxide_qquick_prompt_dialog.cc
    oxide_qquick_software_frame_node.cc
    oxide_qquick_touch_handle_drawable.cc
    oxide_qquick_web_context_menu.cc
    oxide_qquick_web_popup_menu.cc
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
    PUBLIC Qt5::Core Qt5::Qml Qt5::Quick
    PRIVATE Qt5::Gui ${OXIDE_LIB})
set_target_properties(
    ${OXIDE_QUICKLIB} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${OXIDE_LIB_OUTPUT_DIR}
    AUTOMOC TRUE
    SOVERSION ${OXIDE_LIB_VERSION})

install(TARGETS ${OXIDE_QUICKLIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_custom_target(
    test-package-qt-quick
    COMMAND ${CMAKE_COMMAND} -DFILES=$<TARGET_FILE:${OXIDE_QUICKLIB}>
    -DTYPE=FILE -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}
    -P ${CMAKE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)
