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

cmake_policy(VERSION 2.6.0)

foreach(v STAGE_DIR OUTPUT_DIR FILES PROGRAMS DIRECTORIES RUN_QMLAPP_IN RENDERER_PATH SUBPROCESS_DIR)
  if(NOT DEFINED ${v})
    message(FATAL_ERROR "${v} must be defined")
  endif()
endforeach()

foreach(f ${FILES})
  string(REPLACE ":" ";" ARGS ${f})
  list(GET ARGS 0 DIR)
  list(GET ARGS 1 FILENAME)
  file(INSTALL ${FILENAME} DESTINATION ${STAGE_DIR}/${DIR}
       FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
endforeach()

foreach(f ${PROGRAMS})
  string(REPLACE ":" ";" ARGS ${f})
  list(GET ARGS 0 DIR)
  list(GET ARGS 1 FILENAME)
  file(INSTALL ${FILENAME} DESTINATION ${STAGE_DIR}/${DIR}
       FILE_PERMISSIONS
         OWNER_EXECUTE OWNER_WRITE OWNER_READ
         GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ)
endforeach()

foreach(d ${DIRECTORIES})
  string(REPLACE ":" ";" ARGS ${d})
  list(GET ARGS 0 OUTDIR)
  list(GET ARGS 1 DIR)
  file(GLOB FILES "${DIR}/*")
  get_filename_component(DIRNAME ${DIR} NAME)
  foreach(f ${FILES})
    file(INSTALL ${f} DESTINATION ${STAGE_DIR}/${OUTDIR}/${DIRNAME}
         FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
  endforeach()
endforeach()

set(CHROMIUM_LIB_DIR "$(dirname $(readlink -f $0))/${SUBPROCESS_DIR}")
set(LIB_OUTPUT_DIR "$(dirname $(readlink -f $0))")
set(CHROMIUM_PRODUCT_DIR "$(dirname $(readlink -f $0))/${SUBPROCESS_DIR}")
set(OXIDE_RENDERER_NAME ${RENDERER_PATH})
set(QMLPLUGIN_OUTPUT_DIR "$(dirname $(readlink -f $0))/qml")

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
