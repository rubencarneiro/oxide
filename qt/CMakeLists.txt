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

include(GNUInstallDirs)
include(OxideCommon)
include(OxideQtCommon)

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
      ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB_VERSIONED} ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB_UNVERSIONED}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CHROMIUM_PRODUCT_DIR}/chrome_sandbox ${CHROMIUM_PRODUCT_DIR}/chrome-sandbox
    DEPENDS gyp_all
    COMMENT "Running gyp post build commands")

# Allow objects linked by cmake to declare a run-time dependency on the
# main library, built with gyp
add_library(${OXIDE_CORELIB_NAME} SHARED IMPORTED)
add_dependencies(${OXIDE_CORELIB_NAME} gyp_postbuild)
set_target_properties(
    ${OXIDE_CORELIB_NAME}
    PROPERTIES IMPORTED_LOCATION ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB_UNVERSIONED})

add_subdirectory(quick)
if(ENABLE_OXIDEQMLSCENE)
  add_subdirectory(qmlrunner)
endif()
if(ENABLE_TESTS)
  add_subdirectory(tests)
endif()

set(OXIDE_INSTALL_LIB_FILES
    ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB_VERSIONED})
set(OXIDE_INSTALL_LIBEXEC_FILES
    ${CHROMIUM_PRODUCT_DIR}/oxide.pak
    ${CHROMIUM_PRODUCT_DIR}/oxide_100_percent.pak
    ${CHROMIUM_PRODUCT_DIR}/icudtl.dat
    ${CHROMIUM_PRODUCT_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}ffmpegsumo${CMAKE_SHARED_LIBRARY_SUFFIX})
set(OXIDE_INSTALL_LIBEXEC_PROGRAMS
    ${CHROMIUM_PRODUCT_DIR}/${OXIDE_RENDERER_NAME})

install(FILES ${OXIDE_INSTALL_LIB_FILES}
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CHROMIUM_LIB_DIR}/${OXIDE_CORELIB_UNVERSIONED}
        DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${OXIDE_INSTALL_LIBEXEC_FILES}
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(PROGRAMS ${OXIDE_INSTALL_LIBEXEC_PROGRAMS}
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})
install(PROGRAMS ${CHROMIUM_PRODUCT_DIR}/chrome-sandbox
        DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE
                    GROUP_READ WORLD_EXECUTE WORLD_READ SETUID)

list(APPEND PACKAGE_FILES ${OXIDE_INSTALL_LIB_FILES})
list(APPEND PACKAGE_FILES ${OXIDE_INSTALL_LIBEXEC_FILES})
list(APPEND PACKAGE_PROGRAMS ${OXIDE_INSTALL_LIBEXEC_PROGRAMS})
list(APPEND PACKAGE_PROGRAMS ${CHROMIUM_PRODUCT_DIR}/chrome-sandbox)

add_custom_target(
    stage-run-qmlscene
    COMMAND ${CMAKE_COMMAND}
      -DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/run_qmlscene.sh.in
      -DOUTPUT=${OXIDE_STAGE_DIR}/run_qmlscene.sh
      -DRENDERER=${OXIDE_RENDERER_NAME}
      -P ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/stage-run-qmlscene.cmake)
add_custom_target(
    package
    COMMAND ${CMAKE_COMMAND}
      "-DFILES=\"${PACKAGE_FILES}\"" "-DPROGRAMS=\"${PACKAGE_PROGRAMS}\""
      -DSTAGE_DIR=${OXIDE_STAGE_DIR}
      -DOUTPUT_DIR=${CMAKE_BINARY_DIR}/out
      -P ${CMAKE_CURRENT_SOURCE_DIR}/build/scripts/package.cmake
    DEPENDS stage-quick stage-run-qmlscene
    COMMENT "Creating compressed archive")
if(ENABLE_OXIDEQMLSCENE)
  add_dependencies(package stage-oxideqmlscene)
endif()
