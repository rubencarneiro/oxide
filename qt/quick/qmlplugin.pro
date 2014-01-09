TEMPLATE = lib
TARGET = qmloxideplugin
QT += core core-private gui gui-private qml quick quick-private
CONFIG += qt plugin

include($${OXIDE_QMAKE_PATH}/oxide_variables.pri)

uri = com.canonical.Oxide
TARGET.module_name = $$replace(uri, \\., /)

DESTDIR = $${OXIDE_BUILD_OUT}/imports/$${TARGET.module_name}

SOURCES += \
    api/oxideqquickmessagehandler.cc \
    api/oxideqquicknavigationhistory.cc \
    api/oxideqquickoutgoingmessagerequest.cc \
    api/oxideqquickuserscript.cc \
    api/oxideqquickwebcontext.cc \
    api/oxideqquickwebframe.cc \
    api/oxideqquickwebview.cc \
    oxide_qml_plugin.cc \
    oxide_qquick_painted_render_view_node.cc \
    oxide_qquick_render_view_item.cc \
    oxide_qquick_web_popup_menu_delegate.cc \

HEADERS += \
    api/oxideqquickmessagehandler_p.h \
    api/oxideqquickmessagehandler_p_p.h \
    api/oxideqquicknavigationhistory_p.h \
    api/oxideqquicknavigationhistory_p_p.h \
    api/oxideqquickoutgoingmessagerequest_p.h \
    api/oxideqquickoutgoingmessagerequest_p_p.h \
    api/oxideqquickuserscript_p.h \
    api/oxideqquickuserscript_p_p.h \
    api/oxideqquickwebcontext_p.h \
    api/oxideqquickwebcontext_p_p.h \
    api/oxideqquickwebframe_p.h \
    api/oxideqquickwebframe_p_p.h \
    api/oxideqquickwebview_p.h \
    api/oxideqquickwebview_p_p.h \
    oxide_qquick_painted_render_view_node.h \
    oxide_qquick_render_view_item.h \
    oxide_qquick_web_popup_menu_delegate.h

lessThan(QT_MAJOR_VERSION, 5):
else:lessThan(QT_MINOR_VERSION, 2):
else {
SOURCES += oxide_qquick_accelerated_render_view_node.cc
HEADERS += oxide_qquick_accelerated_render_view_node.h

DEFINES += ENABLE_COMPOSITING
}

INCLUDEPATH = $$OXIDE_SRC_ROOT

LIBS += -L$${CHROMIUM_OUT_LIB_DIR} -lOxideQtCore

OTHER_FILES = qmldir

copy2build.target = $${OXIDE_BUILD_OUT}/imports/$${TARGET.module_name}/qmldir
copy2build.depends = $${_PRO_FILE_PWD_}/qmldir
copy2build.commands = $${QMAKE_COPY} $${_PRO_FILE_PWD_}/qmldir $$copy2build.target
QMAKE_EXTRA_TARGETS += copy2build
POST_TARGETDEPS = $$copy2build.target

qmldir.files = qmldir
unix {
    installPath = $$[QT_INSTALL_QML]/$${TARGET.module_name}
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}
