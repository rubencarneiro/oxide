set(QMLRUNNER oxideqmlscene)

set(QMLRUNNER_SRCS main.cc)

include_directories(${Qt5Core_PRIVATE_INCLUDE_DIRS})
if(NOT Qt5Quick_VERSION_STRING VERSION_LESS 5.2.0)
  include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS}
                      ${Qt5Quick_PRIVATE_INCLUDE_DIRS})
endif()

add_executable(${QMLRUNNER} ${QMLRUNNER_SRCS})
set_target_properties(
    ${QMLRUNNER} PROPERTIES
    SKIP_BUILD_RPATH TRUE
    RUNTIME_OUTPUT_DIRECTORY ${OXIDE_BIN_OUTPUT_DIR})

qt5_use_modules(${QMLRUNNER} Core Gui Qml Quick)
