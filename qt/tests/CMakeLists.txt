set(QML_PLUGIN qmloxidetestingplugin)
set(QML_PLUGIN_MODULE_NAME com/canonical/Oxide/Testing)

set(QML_PLUGIN_SRCS qmltests/oxide_qml_testing_plugin.cc)

add_library(${QML_PLUGIN} MODULE ${QML_PLUGIN_SRCS})

set_target_properties(
    ${QML_PLUGIN} PROPERTIES
    SKIP_BUILD_RPATH TRUE AUTOMOC TRUE
    LIBRARY_OUTPUT_DIRECTORY ${OXIDE_QMLPLUGIN_DIR}/${QML_PLUGIN_MODULE_NAME})

qt5_use_modules(${QML_PLUGIN} Core Qml)

foreach(f qmldir TestUtils.js TestUtilsSlave.js TestWebView.qml)
  add_custom_command(
      TARGET ${QML_PLUGIN} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/qmltests/${f}
        ${OXIDE_QMLPLUGIN_DIR}/${QML_PLUGIN_MODULE_NAME})
endforeach()
