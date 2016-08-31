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

include(CommonProperties)
include(CMakePackageConfigHelpers)

function(configure_and_install_package_config_file _inPath)
  get_filename_component(_inFile ${_inPath} NAME)
  string(REGEX REPLACE "Config\\.cmake\\.in$" "" _name ${_inFile})
  set(_configFilename ${_name}Config.cmake)
  set(_versionConfigFilename ${_name}ConfigVersion.cmake)
  configure_package_config_file(
      ${_inPath}
      ${CMAKE_CURRENT_BINARY_DIR}/${_configFilename}
      INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${_name}
      PATH_VARS OXIDE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR OXIDE_INSTALL_LIBEXECDIR
      NO_SET_AND_CHECK_MACRO NO_CHECK_REQUIRED_COMPONENTS_MACRO)

  # Specify VERSION here to make CMake 2.8 happy
  write_basic_package_version_file(
      ${CMAKE_CURRENT_BINARY_DIR}/${_versionConfigFilename}
      VERSION ${PROJECT_VERSION}
      COMPATIBILITY AnyNewerVersion)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_configFilename} ${CMAKE_CURRENT_BINARY_DIR}/${_versionConfigFilename}
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${_name})
endfunction()
