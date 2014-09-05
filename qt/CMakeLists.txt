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

include(CheckCXXSymbolExists)
include(CMakeExpandImportedTargets)
include(GNUInstallDirs)

set(CMAKE_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_LIBDIR}/${OXIDE_SUBPROCESS_DIR})
set(OXIDE_CORELIB_DEV
    ${CMAKE_SHARED_LIBRARY_PREFIX}${OXIDE_CORELIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
set(OXIDE_CORELIB ${OXIDE_CORELIB_DEV}.${OXIDE_CORELIB_SO_VERSION})
set(OXIDE_QMLPLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/imports)
set(OXIDE_BIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/bin)
set(OXIDE_STAGE_DIR ${CMAKE_BINARY_DIR}/out/oxide)

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

find_package(Qt5Core)

if(${Qt5Core_VERSION_STRING} VERSION_LESS "5.3.0")
  find_package(Qt5Gui)
  find_package(Qt5Quick)
  set(CMAKE_REQUIRED_INCLUDES ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS}
      ${Qt5Quick_INCLUDE_DIRS} ${Qt5Core_PRIVATE_INCLUDE_DIRS}
      ${Qt5Gui_PRIVATE_INCLUDE_DIRS} ${Qt5Quick_PRIVATE_INCLUDE_DIRS})
  set(CMAKE_REQUIRED_DEFINITIONS ${Qt5Core_DEFINITIONS} ${Qt5Gui_DEFINITIONS}
      ${Qt5Quick_DEFINITIONS})
  set(CMAKE_REQUIRED_FLAGS -fPIC)
  cmake_expand_imported_targets(
      CMAKE_REQUIRED_LIBRARIES
      LIBRARIES ${Qt5Core_LIBRARIES} ${Qt5Gui_LIBRARIES} ${Qt5Quick_LIBRARIES})
  check_cxx_symbol_exists(
      QSGContext::setSharedOpenGLContext
      QtQuick/private/qsgcontext_p.h
      OXIDE_ENABLE_COMPOSITING)
else()
  set(OXIDE_ENABLE_COMPOSITING TRUE)
endif()

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/run_qmlapp.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/run_qmlapp.sh
    IMMEDIATE @ONLY)
file(INSTALL ${CMAKE_CURRENT_BINARY_DIR}/run_qmlapp.sh
     DESTINATION ${CMAKE_BINARY_DIR}/out/bin
     FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
         GROUP_READ GROUP_EXECUTE
         WORLD_READ WORLD_EXECUTE)

add_custom_target(
    gyp ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB} ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB_DEV}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CHROMIUM_PRODUCT_DIR}/chrome_sandbox ${CHROMIUM_PRODUCT_DIR}/chrome-sandbox
    DEPENDS gyp_all)

# Allow objects linked by cmake to declare a run-time dependency on the
# main library, built with gyp
add_library(${OXIDE_CORELIB_NAME} SHARED IMPORTED)
add_dependencies(${OXIDE_CORELIB_NAME} gyp)
set_target_properties(
    ${OXIDE_CORELIB_NAME}
    PROPERTIES IMPORTED_LOCATION ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB_DEV})

set(OXIDE_INSTALL_LIB_FILES
    ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB}
    ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB_DEV})
set(OXIDE_INSTALL_LIBEXEC_FILES
    ${CHROMIUM_PRODUCT_DIR}/oxide.pak
    ${CHROMIUM_PRODUCT_DIR}/oxide_100_percent.pak
    ${CHROMIUM_PRODUCT_DIR}/oxide_200_percent.pak
    ${CHROMIUM_PRODUCT_DIR}/icudtl.dat
    ${CHROMIUM_PRODUCT_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}ffmpegsumo${CMAKE_SHARED_LIBRARY_SUFFIX})
set(OXIDE_INSTALL_LIBEXEC_PROGRAMS
    ${CHROMIUM_PRODUCT_DIR}/${OXIDE_RENDERER_NAME})
if(DEFINED ENABLE_CHROMEDRIVER AND ENABLE_CHROMEDRIVER EQUAL 1)
  list(APPEND OXIDE_INSTALL_LIBEXEC_PROGRAMS ${CHROMIUM_PRODUCT_DIR}/chromedriver)
endif()

install(FILES ${OXIDE_INSTALL_LIB_FILES}
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${OXIDE_INSTALL_LIBEXEC_FILES}
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(PROGRAMS ${OXIDE_INSTALL_LIBEXEC_PROGRAMS}
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(PROGRAMS ${CHROMIUM_PRODUCT_DIR}/chrome-sandbox
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE
                    GROUP_READ WORLD_EXECUTE WORLD_READ SETUID)
install(DIRECTORY ${CHROMIUM_PRODUCT_DIR}/locales
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})

list(APPEND PACKAGE_FILES ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB})
list(APPEND PACKAGE_FILES ${OXIDE_INSTALL_LIBEXEC_FILES})
list(APPEND PACKAGE_PROGRAMS ${OXIDE_INSTALL_LIBEXEC_PROGRAMS})
list(APPEND PACKAGE_PROGRAMS ${CHROMIUM_PRODUCT_DIR}/chrome-sandbox)
list(APPEND PACKAGE_LOCALES ${CHROMIUM_PRODUCT_DIR}/locales)

macro(oxide_add_package_files)
  list(APPEND PACKAGE_FILES ${ARGN})
  set(PACKAGE_FILES ${PACKAGE_FILES} PARENT_SCOPE)
endmacro()

macro(oxide_add_package_programs)
  list(APPEND PACKAGE_PROGRAMS ${ARGN})
  set(PACKAGE_PROGRAMS ${PACKAGE_PROGRAMS} PARENT_SCOPE)
endmacro()

add_subdirectory(quick)
if(NOT DEFINED ENABLE_OXIDEQMLSCENE OR ENABLE_OXIDEQMLSCENE EQUAL 1)
  add_subdirectory(qmlrunner)
endif()
if(ENABLE_TESTS)
  add_subdirectory(tests)
endif()

add_custom_target(
    package
    COMMAND ${CMAKE_COMMAND}
      "-DFILES=\"${PACKAGE_FILES}\"" "-DPROGRAMS=\"${PACKAGE_PROGRAMS}\""
      "-DLOCALES=\"${PACKAGE_LOCALES}\""
      -DRUN_QMLAPP_IN=${CMAKE_CURRENT_SOURCE_DIR}/run_qmlapp.sh.in
      -DRENDERER_PATH=${OXIDE_RENDERER_NAME}
      -DSTAGE_DIR=${OXIDE_STAGE_DIR}
      -DOUTPUT_DIR=${CMAKE_BINARY_DIR}/out
      -P ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/package.cmake
    COMMENT "Creating compressed archive")
