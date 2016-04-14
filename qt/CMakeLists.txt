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

include(GNUInstallDirs)

include(OxideCommonProperties)
include(OxidePackageConfigHelper)

set(OXIDE_QMLPLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/imports)

set(OXIDE_QUICKLIB OxideQtQuick)

if(CMAKE_CROSSCOMPILING)
  # Dummy targets - not used anywhere, but this stops Qt5CoreConfigExtras.cmake
  # from creating them and checking if the binary exists, which is broken when
  # cross-building because it checks for the target system binary. We need the
  # host system binaries installed, because they are in the same package as the
  # moc in Ubuntu (qtbase5-dev-tools), which is not currently multi-arch
  if(NOT TARGET Qt5::qmake)
    add_executable(Qt5::qmake IMPORTED)
  endif()
  if(NOT TARGET Qt5::rcc)
    add_executable(Qt5::rcc IMPORTED)
  endif()
  if(NOT TARGET Qt5::uic)
    add_executable(Qt5::uic IMPORTED)
  endif()
endif()

find_package(Qt5Core REQUIRED)
# 5.2.0 is required for us to be able to set a share context
find_package(Qt5Quick 5.2.0 REQUIRED)

set(_OXIDEQMLSCENE_DEFAULT OFF)
if(${Qt5Quick_VERSION_STRING} VERSION_LESS "5.3.0")
  set(_OXIDEQMLSCENE_DEFAULT ON)
endif()
option(ENABLE_OXIDEQMLSCENE "Enable the oxideqmlscene binary" ${_OXIDEQMLSCENE_DEFAULT})

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/run_qmlapp.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/run_qmlapp.sh
    IMMEDIATE @ONLY)
file(INSTALL ${CMAKE_CURRENT_BINARY_DIR}/run_qmlapp.sh
     DESTINATION ${OXIDE_BIN_OUTPUT_DIR}
     FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
         GROUP_READ GROUP_EXECUTE
         WORLD_READ WORLD_EXECUTE)

configure_and_install_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/core/api/OxideQtCoreConfig.cmake.in)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/core/api/
        DESTINATION ${OXIDE_INSTALL_INCLUDEDIR}/OxideQtCore
        FILES_MATCHING PATTERN *.h
        PATTERN *_p.h EXCLUDE
        PATTERN includes* EXCLUDE)

set_target_properties(
    ${OXIDE_LIB} PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/core/api/includes
    INTERFACE_LINK_LIBRARIES Qt5::Core)

set(OXIDE_QT_PACKAGE_QMLPLUGIN_DIR qml)

add_subdirectory(qmlplugin)
add_subdirectory(quick)

if(ENABLE_OXIDEQMLSCENE)
  if(NOT ${Qt5Quick_VERSION_STRING} VERSION_LESS "5.4.0")
    message(FATAL_ERROR
            "oxideqmlscene is not supported on Qt 5.4.0 and later - you can "
            "use qmlscene instead")
  elseif(NOT ${Qt5Quick_VERSION_STRING} VERSION_LESS "5.3.0")
    message(WARNING
            "oxideqmlscene is redundant on Qt 5.3.0 and later - you can use "
            "qmlscene instead")
  endif()
  add_subdirectory(qmlrunner)
endif()

if(ENABLE_TESTS)
  add_subdirectory(tests)
endif()

add_custom_target(
    test-package-qt-runner
    COMMAND ${CMAKE_COMMAND}
    -DSRCFILE=${CMAKE_CURRENT_SOURCE_DIR}/run_qmlapp.sh.in
    -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}
    -DQML_DIR=${OXIDE_QT_PACKAGE_QMLPLUGIN_DIR}
    -DPRODUCT_DIR=${OXIDE_PLATFORM_FULLNAME} -DOXIDE_RENDERER=${OXIDE_RENDERER}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/make-test-package-qmlrunner.cmake)

add_custom_target(
    test-package-qt
    DEPENDS test-package-qt-quick test-package-qt-qmlplugin test-package-qt-runner)
