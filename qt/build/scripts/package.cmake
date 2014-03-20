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

foreach(v STAGE_DIR OUTPUT_DIR)
  if(NOT DEFINED ${v})
    message(FATAL_ERROR "${v} must be defined")
  endif()
endforeach()

foreach(f ${FILES})
  file(INSTALL ${f} DESTINATION ${STAGE_DIR}
       FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
endforeach()
foreach(f ${PROGRAMS})
  file(INSTALL ${f} DESTINATION ${STAGE_DIR}
       FILE_PERMISSIONS
         OWNER_EXECUTE OWNER_WRITE OWNER_READ
         GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
endforeach()

get_filename_component(WORKING_DIRECTORY ${STAGE_DIR} PATH)
get_filename_component(STAGE_DIR_NAME ${STAGE_DIR} NAME)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar czf ${OUTPUT_DIR}/oxide-qt.tar.bz2 ${STAGE_DIR_NAME}
    WORKING_DIRECTORY ${WORKING_DIRECTORY})
