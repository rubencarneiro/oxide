TEMPLATE = lib
TARGET = qmloxideplugin
QT += qml quick
CONFIG += qt plugin

include($${OXIDE_QMAKE_PATH}/oxide_variables.pri)

uri = com.canonical.Oxide
TARGET.module_name = $$replace(uri, \\., /)

DESTDIR = $${OXIDE_BUILD_OUT}/imports/$${TARGET.module_name}

SOURCES += oxide_qml_plugin.cc

INCLUDEPATH = $${OXIDE_SRC_ROOT}/qt/lib/api/public
LIBS += -L$${CHROMIUM_OUT_LIB_DIR} -loxide-qt

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
