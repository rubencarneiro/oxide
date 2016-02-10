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

if(DEFINED _OxideCommonProperties_INCLUDED_)
  return()
endif()
set(_OxideCommonProperties_INCLUDED_ TRUE)

include(OxideCommonOptions)

set(OXIDE_PLATFORM_FULLNAME oxide-${OXIDE_PLATFORM})

# This is based on Debian changes to GNUInstallDirs.cmake, and ensures we
# install headers in to a multi-arch location
set(_INCLUDEDIR_DEFAULT ${CMAKE_INSTALL_INCLUDEDIR})
if(DEFINED _OxideCommonProperties_LAST_CMAKE_INSTALL_PREFIX)
  set(__LAST_INCLUDEDIR_DEFAULT ${CMAKE_INSTALL_INCLUDEDIR})
endif()
if(CMAKE_SYSTEM_NAME MATCHES "^(Linux|kFreeBSD|GNU)$" AND
     NOT CMAKE_CROSSCOMPILING AND
     EXISTS "/etc/debian_version" AND
     CMAKE_LIBRARY_ARCHITECTURE)
  if("${CMAKE_INSTALL_PREFIX}" MATCHES "^/usr/?$")
    set(_INCLUDEDIR_DEFAULT "${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_LIBRARY_ARCHITECTURE}")
  endif()
  if(DEFINED _OxideCommonProperties_LAST_CMAKE_INSTALL_PREFIX
       AND "${_OxideCommonProperties_LAST_CMAKE_INSTALL_PREFIX}" MATCHES "^/usr/?$")
    set(__LAST_INCLUDEDIR_DEFAULT "lib/${CMAKE_LIBRARY_ARCHITECTURE}")
  endif()
endif()
if(NOT DEFINED OXIDE_INSTALL_INCLUDEDIR)
  set(OXIDE_INSTALL_INCLUDEDIR ${_INCLUDEDIR_DEFAULT}/${OXIDE_PLATFORM_FULLNAME} CACHE PATH "C header files (include)")
elseif(DEFINED __LAST_INCLUDEDIR_DEFAULT
         AND "${__LAST_INCLUDEDIR_DEFAULT}" STREQUAL "${CMAKE_INSTALL_INCLUDEDIR}")
  set_property(CACHE OXIDE_INSTALL_INCLUDEDIR PROPERTY VALUE "${_INCLUDEDIR_DEFAULT}")
endif()
set(_OxideCommonProperties_LAST_CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE INTERNAL "CMAKE_INSTALL_PREFIX during last run")
unset(_INCLUDEDIR_DEFAULT)
unset(__LAST_INCLUDEDIR_DEFAULT)
