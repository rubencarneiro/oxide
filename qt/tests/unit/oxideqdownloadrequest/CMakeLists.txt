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

add_executable(tst_oxideqdownloadrequest tst_oxideqdownloadrequest.cc)
target_link_libraries(tst_oxideqdownloadrequest
                      PRIVATE
                      ${OXIDE_LIB}
                      ${Qt5Core_LIBRARIES}
                      ${Qt5Gui_LIBRARIES}
                      ${Qt5Test_LIBRARIES})
set_target_properties(tst_oxideqdownloadrequest PROPERTIES
                      AUTOMOC TRUE)

add_test(NAME tst_oxideqdownloadrequest
         COMMAND ${OXIDE_BIN_OUTPUT_DIR}/run_qmlapp.sh
                 ${CMAKE_CURRENT_BINARY_DIR}/tst_oxideqdownloadrequest)
