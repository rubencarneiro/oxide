if(CMAKE_CROSSCOMPILING)
  # QT_MOC_EXECUTABLE is set by Qt5CoreConfigExtras, but it sets it to
  # the target executable rather than the host executable, which is no
  # use for cross-compiling. For cross-compiling, we have a guess and
  # override it ourselves
  find_program(
      QT_MOC_EXECUTABLE moc
      PATHS /usr/lib/qt5/bin
        /usr/lib/${OXIDE_LIBRARY_HOST_ARCHITECTURE}/qt5/bin
      NO_DEFAULT_PATH)
  if(QT_MOC_EXECUTABLE STREQUAL "QT_MOC_EXECUTABLE-NOTFOUND")
    message(FATAL_ERROR "Can't find a moc executable for the host arch")
  endif()
  add_executable(Qt5::moc IMPORTED)
  set_target_properties(Qt5::moc PROPERTIES
      IMPORTED_LOCATION "${QT_MOC_EXECUTABLE}")
else()
  # This should be enough to initialize QT_MOC_EXECUTABLE
  find_package(Qt5Core)
endif()

set(OXIDE_CORELIB_NAME OxideQtCore)
set(OXIDE_CORELIB_SO_VERSION 0)
set(OXIDE_SUBPROCESS_DIR oxide-qt)
set(OXIDE_RENDERER_NAME oxide-renderer)

list(APPEND OXIDE_GYP_EXTRA_ARGS -Doxide_core_name=${OXIDE_CORELIB_NAME})
list(APPEND OXIDE_GYP_EXTRA_ARGS -Doxide_core_so_version=${OXIDE_CORELIB_SO_VERSION})
list(APPEND OXIDE_GYP_EXTRA_ARGS -Doxide_subprocess_dir=${OXIDE_SUBPROCESS_DIR})
list(APPEND OXIDE_GYP_EXTRA_ARGS -Doxide_renderer_name=${OXIDE_RENDERER_NAME})
list(APPEND OXIDE_GYP_EXTRA_ARGS -Dqt_moc_executable=${QT_MOC_EXECUTABLE})
