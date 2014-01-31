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

if(CMAKE_CROSSCOMPILING)
  # Dummy targets - not used anywhere, but this stops Qt5CoreConfigExtras.cmake
  # from creating them and checking if the binary exists, which is broken when
  # cross-building because it checks for the target system binary. We need the
  # host system binaries installed, because they are in the same package as the
  # moc in Ubuntu (qtbase5-dev-tools), which is not currently multi-arch
  add_executable(Qt5::qmake IMPORTED)
  add_executable(Qt5::rcc IMPORTED)
  add_executable(Qt5::uic IMPORTED)
endif()

set(CMAKE_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_LIBDIR}/${OXIDE_SUBPROCESS_DIR})
set(OXIDE_CORELIB_UNVERSIONED
    ${CMAKE_SHARED_LIBRARY_PREFIX}${OXIDE_CORELIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX})
set(OXIDE_CORELIB_VERSIONED ${OXIDE_CORELIB_UNVERSIONED}.${OXIDE_CORELIB_SO_VERSION})
set(OXIDE_QMLPLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/imports)
set(OXIDE_BIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/bin)

# Allow objects linked by cmake to declare a run-time dependency on the
# main library, built with gyp
add_library(${OXIDE_CORELIB_NAME} SHARED IMPORTED)
set_target_properties(
    ${OXIDE_CORELIB_NAME}
    PROPERTIES IMPORTED_LOCATION ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB_UNVERSIONED})

find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Qml)
find_package(Qt5Quick)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/run_qmlscene.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/run_qmlscene.sh
    IMMEDIATE @ONLY)
file(INSTALL ${CMAKE_CURRENT_BINARY_DIR}/run_qmlscene.sh
     DESTINATION ${CMAKE_BINARY_DIR}/out
     FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
         GROUP_READ GROUP_EXECUTE
         WORLD_READ WORLD_EXECUTE)

add_custom_target(
    gyp_postbuild ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB_VERSIONED} ${CHROMIUM_LIB_DIR}${OXIDE_CORELIB_UNVERSIONED}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CHROMIUM_PRODUCT_DIR}/chrome_sandbox ${CHROMIUM_PRODUCT_DIR}/chrome-sandbox
    DEPENDS gyp_all
    COMMENT "Running gyp post build commands")

add_subdirectory(quick)
if(ENABLE_OXIDEQMLSCENE)
  add_subdirectory(qmlrunner)
endif()
if(ENABLE_TESTS)
  add_subdirectory(tests)
endif()

install(FILES ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB_VERSIONED}
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB_UNVERSIONED}
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CHROMIUM_PRODUCT_DIR}/oxide.pak
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES ${CHROMIUM_PRODUCT_DIR}/oxide_100_percent.pak
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES ${CHROMIUM_PRODUCT_DIR}/icudtl.dat
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(FILES ${CHROMIUM_PRODUCT_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}ffmpegsumo${CMAKE_SHARED_LIBRARY_SUFFIX}
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(PROGRAMS ${CHROMIUM_PRODUCT_DIR}/chrome-sandbox
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE
                    GROUP_READ WORLD_EXECUTE WORLD_READ SETUID)
install(PROGRAMS ${CHROMIUM_PRODUCT_DIR}/${OXIDE_RENDERER_NAME}
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
