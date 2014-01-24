set(PLUGIN qmloxideplugin)
set(PLUGIN_MODULE_NAME com/canonical/Oxide)

set(PLUGIN_SRCS
    api/oxideqquickglobals.cc
    api/oxideqquickmessagehandler.cc
    api/oxideqquicknavigationhistory.cc
    api/oxideqquickoutgoingmessagerequest.cc
    api/oxideqquickuserscript.cc
    api/oxideqquickwebcontext.cc
    api/oxideqquickwebframe.cc
    api/oxideqquickwebview.cc
    oxide_qml_plugin.cc
    oxide_qquick_painted_render_view_node.cc
    oxide_qquick_render_view_item.cc
    oxide_qquick_web_popup_menu_delegate.cc)

include_directories(${CMAKE_SOURCE_DIR}
                    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
                    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
                    ${Qt5Quick_PRIVATE_INCLUDE_DIRS})

if(NOT ${Qt5Quick_VERSION_STRING} VERSION_LESS 5.2.0)
  list(APPEND PLUGIN_SRCS oxide_qquick_accelerated_render_view_node.cc)
  add_definitions(-DENABLE_COMPOSITING)
endif()

add_library(${PLUGIN} MODULE ${PLUGIN_SRCS})
add_dependencies(${PLUGIN} gyp_all)
set_target_properties(
    ${PLUGIN} PROPERTIES
    SKIP_BUILD_RPATH TRUE AUTOMOC TRUE
    LIBRARY_OUTPUT_DIRECTORY ${OXIDE_QMLPLUGIN_DIR}/${PLUGIN_MODULE_NAME})

qt5_use_modules(${PLUGIN} Core Gui Qml Quick)
target_link_libraries(${PLUGIN} OxideLib)

add_custom_command(
    TARGET ${PLUGIN} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/qmldir ${OXIDE_QMLPLUGIN_DIR}/${PLUGIN_MODULE_NAME}
    COMMENT "Copying qmldir to output directory")
