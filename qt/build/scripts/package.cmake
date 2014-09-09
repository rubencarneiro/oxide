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

foreach(v STAGE_DIR OUTPUT_DIR LOCALES RUN_QMLAPP_IN RENDERER_PATH)
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

file(GLOB LOCALE_FILES "${LOCALES}/*.pak")
foreach(f ${LOCALE_FILES})
  file(INSTALL ${f} DESTINATION ${STAGE_DIR}/locales
       FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
endforeach()

set(CHROMIUM_LIB_DIR "$(dirname $(readlink -f $0))")
set(CHROMIUM_PRODUCT_DIR "$(dirname $(readlink -f $0))")
set(OXIDE_RENDERER_NAME ${RENDERER_PATH})
set(OXIDE_QMLPLUGIN_OUTPUT_DIR "$(dirname $(readlink -f $0))")

configure_file(${RUN_QMLAPP_IN} ${STAGE_DIR}/.tmp/run_qmlapp.sh IMMEDIATE @ONLY)
file(INSTALL ${STAGE_DIR}/.tmp/run_qmlapp.sh DESTINATION ${STAGE_DIR}
     PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                 GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
execute_process(COMMAND ${CMAKE_COMMAND} -E remove_directory ${STAGE_DIR}/.tmp)

get_filename_component(WORKING_DIRECTORY ${STAGE_DIR} PATH)
get_filename_component(STAGE_DIR_NAME ${STAGE_DIR} NAME)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar czf ${OUTPUT_DIR}/oxide-qt.tar.bz2 ${STAGE_DIR_NAME}
    WORKING_DIRECTORY ${WORKING_DIRECTORY})
