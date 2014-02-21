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

foreach(v MAKE GYP_DIR CC CXX)
  if(NOT DEFINED ${v})
    message(FATAL_ERROR "Need to define ${v}")
  endif()
endforeach()

set(ENV{CFLAGS})
set(ENV{CXXFLAGS})
set(ENV{CPPFLAGS})
set(ENV{LDFLAGS})
set(ENV{CC} ${CC})
set(ENV{CXX} ${CXX})

foreach(v CC_host CXX_host)
  if(DEFINED ${v})
    set(ENV{${v}} ${${v}})
  endif()
endforeach()

set(MAKE_COMMAND ${MAKE} -C ${GYP_DIR} -f Makefile all)
if(DEFINED BUILDTYPE)
  list(APPEND MAKE_COMMAND BUILDTYPE=${BUILDTYPE})
endif()
if(CMAKE_VERBOSE_MAKEFILE)
  list(APPEND MAKE_COMMAND V=1)
endif()

execute_process(COMMAND ${MAKE_COMMAND} RESULT_VARIABLE _RESULT)
if(NOT _RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to build gyp target")
endif()
