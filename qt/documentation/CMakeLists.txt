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

find_program(QDOC_EXE qdoc)
if(QDOC_EXE STREQUAL "QDOC_EXE-NOTFOUND")
  message(FATAL "Cannot find qdoc executable")
endif()

# CMake doesn't seem to provide a way to get at QT_INSTALL_DOCS
if(NOT CMAKE_CROSSCOMPILING)
  execute_process(COMMAND ${QMAKE_EXECUTABLE} -qt5 -query QT_INSTALL_DOCS
                  RESULT_VARIABLE _RESULT
                  OUTPUT_VARIABLE QT_INSTALL_DOCS
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT _RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to determine QT_INSTALL_DOCS from qmake")
  endif()
else()
  set(QT_INSTALL_DOCS "/usr/share/qt5/doc")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/oxideqt.qdocconf.in
               ${CMAKE_CURRENT_BINARY_DIR}/oxideqt.qdocconf
               IMMEDIATE @ONLY)

add_custom_target(doc 
                  COMMAND ${QDOC_EXE} -qt5 ${CMAKE_CURRENT_BINARY_DIR}/oxideqt.qdocconf
                  COMMENT "Building the documentation")

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html
        DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/${OXIDE_PLATFORM_FULLNAME}
        OPTIONAL)
