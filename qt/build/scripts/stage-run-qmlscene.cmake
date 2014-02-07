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

foreach(v INPUT OUTPUT RENDERER)
  if(NOT DEFINED ${v})
    message(FATAL_ERROR "${v} must be defined")
  endif()
endforeach()

set(CHROMIUM_LIB_DIR "$(dirname $(readlink -f $0))")
set(CHROMIUM_PRODUCT_DIR "$(dirname $(readlink -f $0))")
set(OXIDE_RENDERER_NAME ${RENDERER})
set(OXIDE_QMLPLUGIN_OUTPUT_DIR "$(dirname $(readlink -f $0))")

get_filename_component(OUTPUT_DIR ${OUTPUT} PATH)
get_filename_component(OUTPUT_NAME ${OUTPUT} NAME)
configure_file(${INPUT} ${OUTPUT_DIR}/tmp/${OUTPUT_NAME} IMMEDIATE @ONLY)

file(INSTALL ${OUTPUT_DIR}/tmp/${OUTPUT_NAME} DESTINATION ${OUTPUT_DIR}
     PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                 GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${OUTPUT_DIR}/tmp)
