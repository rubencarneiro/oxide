find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Qml)
find_package(Qt5Quick)

set(OXIDE_LIB_VERSIONED ${OXIDE_LIB}.${QT_SO_VERSION})
set(OXIDE_QMLPLUGIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/qml)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/run_qmlscene.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/run_qmlscene.sh
    IMMEDIATE @ONLY)
file(INSTALL ${CMAKE_CURRENT_BINARY_DIR}/run_qmlscene.sh
     DESTINATION ${CMAKE_BINARY_DIR}
     FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
         GROUP_READ GROUP_EXECUTE
         WORLD_READ WORLD_EXECUTE)

add_custom_target(
    create_symlinks ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${OXIDE_LIB_VERSIONED} ${OXIDE_LIB}
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CHROMIUM_PRODUCT_DIR}/chrome_sandbox ${CHROMIUM_PRODUCT_DIR}/chrome-sandbox
    DEPENDS gyp_all
    COMMENT "Running gyp post build commands")

add_subdirectory(quick)
if(${WANT_OXIDEQMLSCENE})
  add_subdirectory(qmlrunner)
endif()
