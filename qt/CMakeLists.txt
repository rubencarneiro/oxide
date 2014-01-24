find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Qml)
find_package(Qt5Quick)

set(OXIDE_LIB_VERSIONED ${OXIDE_LIB}.${QT_SO_VERSION})

add_custom_target(
    create_symlinks ALL
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${OXIDE_LIB_VERSIONED} ${OXIDE_LIB}
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CHROMIUM_PRODUCT_DIR}/chrome_sandbox ${CHROMIUM_PRODUCT_DIR}/chrome-sandbox
    DEPENDS gyp_all
    COMMENT "Running gyp post build commands")

add_subdirectory(quick)
