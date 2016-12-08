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

set(OXIDE_UITKLIB_SRCS
    api/oxideubuntuwebcontextmenu.cc
    api/oxideubuntuwebcontextmenuitem.cc
    api/oxideubuntuwebview.cc
    resources.qrc
    uitk_auxiliary_ui_factory.cc
    uitk_contents_view.cc
    uitk_web_context_menu.cc
    uitk_web_popup_menu.cc)

add_library(${OXIDE_UITKLIB} SHARED ${OXIDE_UITKLIB_SRCS})
target_compile_definitions(
    ${OXIDE_UITKLIB} PRIVATE -DOXIDE_UITK_IMPLEMENTATION)
target_include_directories(${OXIDE_UITKLIB}
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/api/includes)
target_link_libraries(
    ${OXIDE_UITKLIB}
    PUBLIC ${OXIDE_QUICKLIB}
    PRIVATE ${OXIDE_CORELIB} ${Qt5Core_LIBRARIES} ${Qt5Qml_LIBRARIES}
            ${Qt5Quick_LIBRARIES} ${Qt5Gui_LIBRARIES})
set_target_properties(
    ${OXIDE_UITKLIB} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${OXIDE_LIB_OUTPUT_DIR}
    AUTOMOC TRUE
    AUTORCC TRUE
    SOVERSION ${OXIDE_LIB_VERSION})

install(TARGETS ${OXIDE_UITKLIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/api/
        DESTINATION ${OXIDE_INSTALL_INCLUDEDIR}/OxideQtQuick
        FILES_MATCHING PATTERN *.h
        PATTERN *_p.h EXCLUDE
        PATTERN includes* EXCLUDE)

add_custom_target(
    test-package-uitk-lib
    COMMAND ${CMAKE_COMMAND} -DFILES=$<TARGET_FILE:${OXIDE_UITKLIB}>
    -DTYPE=FILE -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}
    -P ${OXIDE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)

#get_library_filename(OXIDE_UITKLIB_FILENAME ${OXIDE_UITKLIB} ${OXIDE_LIB_VERSION})
#configure_and_install_package_config_file(
#    ${CMAKE_CURRENT_SOURCE_DIR}/api/OxideUbuntuUITKConfig.cmake.in)
