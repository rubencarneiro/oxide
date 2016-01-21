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

if(DEFINED _OxidePackageConfigHelper_INCLUDED_)
  return()
endif()
set(_OxidePackageConfigHelper_INCLUDED_ TRUE)

include(CMakePackageConfigHelpers)
include(OxideCommonProperties)

function(configure_and_install_package_config_file _inPath)
  get_filename_component(_inFile ${_inPath} NAME)
  string(REGEX REPLACE "\\.in$" "" _outFile ${_inFile})
  configure_package_config_file(
      ${_inPath}
      ${CMAKE_CURRENT_BINARY_DIR}/${_outFile}
      INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${OXIDE_PLATFORM_FULLNAME}
      PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
      NO_SET_AND_CHECK_MACRO NO_CHECK_REQUIRED_COMPONENTS_MACRO)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_outFile}
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${OXIDE_PLATFORM_FULLNAME})
endfunction()
