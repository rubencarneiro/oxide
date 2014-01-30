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

foreach(v CHROMIUM_PRODUCT_DIR CHROMIUM_LIB_DIR SOURCE_DIR SERVER_DIR BINARY_DIR TEST_DIR TEST_NAME PYTHON)
  if(NOT DEFINED ${v})
    message(FATAL_ERROR "Must specify ${v}")
  endif()
endforeach()

if(USE_DATA_DIR)
  set(_T -t)
endif()

set(ENV{OXIDE_NO_SANDBOX} 1)
set(ENV{OXIDE_SUBPROCESS_PATH} ${CHROMIUM_PRODUCT_DIR}/oxide-renderer)
set(ENV{LD_LIBRARY_PATH} ${CHROMIUM_LIB_DIR})

set(_COMMAND ${PYTHON} ${SOURCE_DIR}/qt/tests/runtests.py -s ${SERVER_DIR} -p 8080 ${_T} --
    ${BINARY_DIR}/qt/bin/qmltest -name ${TEST_NAME} -import ${BINARY_DIR}/qt/imports -input ${TEST_DIR})
message("Running ${_COMMAND}")
execute_process(COMMAND ${_COMMAND} RESULT_VARIABLE _RESULT)
if(NOT ${_RESULT} EQUAL 0)
  message(FATAL_ERROR "Tests ${TEST_NAME} failed!")
endif()
