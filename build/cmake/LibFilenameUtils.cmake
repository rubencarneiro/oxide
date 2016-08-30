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

macro(get_library_filename _out _name _version)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(${_out} ${CMAKE_SHARED_LIBRARY_PREFIX}${_name}${CMAKE_SHARED_LIBRARY_SUFFIX}.${_version})
  else()
    message(FATAL_ERROR "get_library_filename is not implemented for this platform")
  endif()
endmacro()

macro(get_library_namelink_filename _out _name)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(${_out} ${CMAKE_SHARED_LIBRARY_PREFIX}${_name}${CMAKE_SHARED_LIBRARY_SUFFIX})
  else()
    message(FATAL_ERROR "get_library_namelink_filename is not implemented for this platform")
  endif()
endmacro()

macro(get_library_extension _out _version)
  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(${_out} ${CMAKE_SHARED_LIBRARY_SUFFIX}.${_version})
  else()
    message(FATAL_ERROR "get_library_extension is not implemented for this platform")
  endif()
endmacro()
