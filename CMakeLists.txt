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

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/cmake)

include(ProjectInit)
oxide_project_init()

include(CheckCXXCompilerFlag)
include(CheckIncludeFileCXX)
include(GNUInstallDirs)

include(ChromiumBuildShim)
include(CommonProperties)
include(LibFilenameUtils)
include(Options)

# Define variables used throughout the build
set(OXIDE_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out)
set(OXIDE_BIN_OUTPUT_DIR ${OXIDE_OUTPUT_DIR}/bin)
set(OXIDE_LIB_OUTPUT_DIR ${OXIDE_OUTPUT_DIR}/lib)
set(OXIDE_PACKAGE_STAGE_DIR ${CMAKE_BINARY_DIR}/_package_stage)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # Use gold everywhere
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fuse-ld=gold")
endif()

# Required for out-of-tree builds
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
# We have a script for loading apps from the tree
set(CMAKE_SKIP_BUILD_RPATH TRUE)
include_directories(${OXIDE_SOURCE_DIR})

# Allow the version number to be used in the build
string(REGEX REPLACE "([^\\.]+)\\.([^\\.]+)\\.([^\\.]+)" "\\1" PROJECT_VERSION_MAJOR ${PROJECT_VERSION})
string(REGEX REPLACE "([^\\.]+)\\.([^\\.]+)\\.([^\\.]+)" "\\2" PROJECT_VERSION_MINOR ${PROJECT_VERSION})
string(REGEX REPLACE "([^\\.]+)\\.([^\\.]+)\\.([^\\.]+)" "\\3" PROJECT_VERSION_PATCH ${PROJECT_VERSION})

add_definitions(-DOXIDE_GETTEXT_DOMAIN="${GETTEXT_PACKAGE}")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_definitions(-DOS_LINUX)
else()
  message(FATAL_ERROR "Unsupported CMAKE_SYSTEM_NAME")
endif()

# Check we have a new enough compiler
if(CMAKE_COMPILER_ID STREQUAL "GNU" AND
   CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
  message(FATAL_ERROR "Oxide requires at least GCC 4.8 to build")
endif()
CHECK_CXX_COMPILER_FLAG("-std=c++11" _COMPILER_HAS_CXX11_RESULT)
if(NOT _COMPILER_HAS_CXX11_RESULT)
  message(FATAL_ERROR "Oxide requires a compiler that supports -std=c++11")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Import the configuration for this build
include(${OXIDE_SOURCE_DIR}/${OXIDE_PLATFORM}/config.cmake)

# Set extra variables that depend on the configuration
get_library_filename(OXIDE_LIB_FILENAME ${OXIDE_LIB} ${OXIDE_LIB_VERSION})
get_library_namelink_filename(OXIDE_LIB_FILENAME_DEV ${OXIDE_LIB})

if(ENABLE_COMPONENT_BUILD)
  link_directories(${CHROMIUM_LIB_DIR})
endif()

if(ENABLE_TESTS)
  if(ENABLE_CHROMIUM_TESTS)
    add_test(NAME base_unittests
             COMMAND ${CHROMIUM_PRODUCT_DIR}/base_unittests)
    add_test(NAME crypto_unittests
             COMMAND ${CHROMIUM_PRODUCT_DIR}/crypto_unittests)
    add_test(NAME ipc_tests
             COMMAND ${CHROMIUM_PRODUCT_DIR}/ipc_tests)
  endif()
  add_test(NAME shared_unittests
           COMMAND ${CHROMIUM_PRODUCT_DIR}/oxide_shared_unittests)
endif()

add_chromium_build_all_target(build_all)

# Copy chrome_sandbox to chrome-sandbox
add_custom_target(
    copy_chrome_sandbox ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CHROMIUM_PRODUCT_DIR}/chrome_sandbox ${CHROMIUM_PRODUCT_DIR}/chrome-sandbox
    DEPENDS build_all)

# Allow objects linked by cmake to declare a run-time dependency on the
# main library, built with gn / ninja
add_library(${OXIDE_LIB} SHARED IMPORTED)
add_dependencies(${OXIDE_LIB} build_all)
set_target_properties(
    ${OXIDE_LIB} PROPERTIES
    IMPORTED_LOCATION ${CHROMIUM_LIB_DIR}/${OXIDE_LIB_FILENAME})

file(GLOB_RECURSE I18N_SRC_FILES
     RELATIVE ${OXIDE_SOURCE_DIR}
     ${OXIDE_PLATFORM}/**.cc shared/**.cc)
list(SORT I18N_SRC_FILES)

set(_OXIDE_INSTALL_LIB ${CHROMIUM_LIB_DIR}/${OXIDE_LIB_FILENAME})
set(_OXIDE_INSTALL_FILES
    ${CHROMIUM_PRODUCT_DIR}/oxide.pak
    ${CHROMIUM_PRODUCT_DIR}/oxide_100_percent.pak
    ${CHROMIUM_PRODUCT_DIR}/oxide_200_percent.pak
    ${CHROMIUM_PRODUCT_DIR}/icudtl.dat
    ${CHROMIUM_LIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}ffmpeg${CMAKE_SHARED_LIBRARY_SUFFIX}
    ${CHROMIUM_PRODUCT_DIR}/natives_blob.bin
    ${CHROMIUM_PRODUCT_DIR}/snapshot_blob.bin)
set(_OXIDE_INSTALL_PROGRAMS ${CHROMIUM_PRODUCT_DIR}/${OXIDE_RENDERER})
set(_CHROME_SANDBOX ${CHROMIUM_PRODUCT_DIR}/chrome-sandbox)
set(_OXIDE_INSTALL_LOCALEDIR ${CHROMIUM_PRODUCT_DIR}/chromium_l10n)

install(FILES ${_OXIDE_INSTALL_LIB} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(CODE
        "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${OXIDE_LIB_FILENAME} ${OXIDE_LIB_FILENAME_DEV}
        WORKING_DIRECTORY \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})")
install(FILES ${_OXIDE_INSTALL_FILES} DESTINATION ${OXIDE_INSTALL_LIBEXECDIR})
install(PROGRAMS ${_OXIDE_INSTALL_PROGRAMS}
        DESTINATION ${OXIDE_INSTALL_LIBEXECDIR})
install(PROGRAMS ${_CHROME_SANDBOX}
        DESTINATION ${OXIDE_INSTALL_LIBEXECDIR}
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE
                    GROUP_READ WORLD_EXECUTE WORLD_READ SETUID)
install(DIRECTORY ${_OXIDE_INSTALL_LOCALEDIR}
        DESTINATION ${OXIDE_INSTALL_LIBEXECDIR})
set(_COMPONENT_BUILD_INSTALL_EXCLUDES
    "buildtools;chromium_l10n;gen;host_;Mojo Applications;obj;pyproto;resources;ui")
file(RELATIVE_PATH _COMPONENT_LIB_REL ${CHROMIUM_PRODUCT_DIR} ${CHROMIUM_LIB_DIR})
if(ENABLE_COMPONENT_BUILD)
  set(_COMPONENT_BUILD_INSTALL_PARAMS
      DIRECTORY ${CHROMIUM_LIB_DIR}/
      DESTINATION ${OXIDE_INSTALL_LIBEXECDIR}/${_COMPONENT_LIB_REL}
      FILES_MATCHING PATTERN "*${CMAKE_SHARED_LIBRARY_SUFFIX}")
  foreach(e ${_COMPONENT_BUILD_INSTALL_EXCLUDES})
    list(APPEND _COMPONENT_BUILD_INSTALL_PARAMS PATTERN "${e}" EXCLUDE)
  endforeach()
  install(${_COMPONENT_BUILD_INSTALL_PARAMS})
endif()

add_subdirectory(po)
add_subdirectory(${OXIDE_PLATFORM})

add_custom_target(
    test-package-common-libs
    COMMAND ${CMAKE_COMMAND} -DFILES=${_OXIDE_INSTALL_LIB}
    -DTYPE=FILE -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}
    -P ${OXIDE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)
add_custom_target(
    test-package-common-programs
    COMMAND ${CMAKE_COMMAND}
    "-DFILES=\"${_OXIDE_INSTALL_PROGRAMS};${_CHROME_SANDBOX}\""
    -DTYPE=PROGRAM
    -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}/${OXIDE_PLATFORM_FULLNAME}
    -P ${OXIDE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)
add_custom_target(
    test-package-common-files
    COMMAND ${CMAKE_COMMAND} "-DFILES=\"${_OXIDE_INSTALL_FILES}\""
    -DTYPE=FILE
    -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}/${OXIDE_PLATFORM_FULLNAME}
    -P ${OXIDE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)
add_custom_target(
    test-package-common-locales
    COMMAND ${CMAKE_COMMAND} -DFILES=${_OXIDE_INSTALL_LOCALEDIR}
    -DTYPE=DIRECTORY
    -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}/${OXIDE_PLATFORM_FULLNAME}
    -P ${OXIDE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)
if(ENABLE_COMPONENT_BUILD)
  add_custom_target(
      test-package-common-component-libs
      COMMAND ${CMAKE_COMMAND} -DFILES=${CHROMIUM_LIB_DIR}/
      -DMATCH_PATTERN="*${CMAKE_SHARED_LIBRARY_SUFFIX}" -DTYPE=DIRECTORY
      -DDESTINATION=${OXIDE_PACKAGE_STAGE_DIR}/${OXIDE_PLATFORM_FULLNAME}/${_COMPONENT_LIB_REL}
      "-DEXCLUDES=\"${_COMPONENT_BUILD_INSTALL_EXCLUDES}\""
      -P ${OXIDE_SOURCE_DIR}/build/scripts/test-package-helper.cmake)
endif()

add_custom_target(
    test-package-common
    DEPENDS test-package-common-libs
            test-package-common-programs
            test-package-common-files
            test-package-common-locales)
if(ENABLE_COMPONENT_BUILD)
  add_dependencies(test-package-common test-package-common-component-libs)
endif()

add_custom_target(
    test-package
    COMMAND ${CMAKE_COMMAND} -E
    tar czvf ${OXIDE_OUTPUT_DIR}/${OXIDE_PLATFORM_FULLNAME}.tar.gz *
    WORKING_DIRECTORY ${OXIDE_PACKAGE_STAGE_DIR}
    DEPENDS test-package-common test-package-${OXIDE_PLATFORM})

run_generate_ninja()
