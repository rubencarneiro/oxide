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

cmake_policy(VERSION 2.6.0)

set(OXIDE_LIB_OUTPUT_DIR "$(dirname $(readlink -f $0))")
set(CHROMIUM_PRODUCT_DIR "$(dirname $(readlink -f $0))/${PRODUCT_DIR}")
set(OXIDE_QMLPLUGIN_OUTPUT_DIR "$(dirname $(readlink -f $0))/${QML_DIR}")

configure_file(${SRCFILE} ${DESTINATION}/.tmp/run_qmlapp.sh IMMEDIATE @ONLY)
file(INSTALL FILES ${DESTINATION}/.tmp/run_qmlapp.sh TYPE PROGRAM
     DESTINATION ${DESTINATION})
execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${DESTINATION}/.tmp)
