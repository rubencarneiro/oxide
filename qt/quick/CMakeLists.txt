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

qt5_wrap_cpp(MOC_EXTRA
             api/oxideqquickuserscript_p_p.h
             api/oxideqquickwebcontext_p_p.h
             api/oxideqquickwebcontextdelegateworker_p_p.h)

set(OXIDE_QUICKLIB_SRCS
    api/oxideqquickcookiemanager.cc
    api/oxideqquickglobal.cc
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
    oxide_qquick_alert_dialog_delegate.cc
    oxide_qquick_before_unload_dialog_delegate.cc
    oxide_qquick_confirm_dialog_delegate.cc
    oxide_qquick_file_picker_delegate.cc
    oxide_qquick_init.cc
    oxide_qquick_javascript_dialog_delegate.cc
    oxide_qquick_prompt_dialog_delegate.cc
    oxide_qquick_software_frame_node.cc
    oxide_qquick_web_popup_menu_delegate.cc
    ${MOC_EXTRA})

if(OXIDE_ENABLE_COMPOSITING)
  list(APPEND OXIDE_QUICKLIB_SRCS oxide_qquick_accelerated_frame_node.cc)
endif()

add_library(${OXIDE_QUICKLIB} SHARED ${OXIDE_QUICKLIB_SRCS})
qt5_use_modules(${OXIDE_QUICKLIB} Core Gui Qml Quick)
if(OXIDE_ENABLE_COMPOSITING)
  target_compile_definitions(${OXIDE_QUICKLIB} PRIVATE -DENABLE_COMPOSITING)
endif()
target_include_directories(
    ${OXIDE_QUICKLIB} PRIVATE
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Quick_PRIVATE_INCLUDE_DIRS})
target_link_libraries(${OXIDE_QUICKLIB} ${OXIDE_CORELIB_NAME})
set_target_properties(
    ${OXIDE_QUICKLIB} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}
    AUTOMOC TRUE
    SOVERSION ${OXIDE_CORELIB_SO_VERSION})

install(TARGETS ${OXIDE_QUICKLIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})
