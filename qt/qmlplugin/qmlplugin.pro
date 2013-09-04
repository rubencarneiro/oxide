TEMPLATE = lib
TARGET = qmloxideplugin
QT += qml quick
CONFIG += qt plugin

DEPTH = ../..

uri = com.canonical.Oxide

SOURCES += oxide_qml_plugin.cc

CHROMIUM_OUTPUT_DIR = $${DEPTH}/chromium/src/out
equals(OXIDE_DEBUG, "1") {
    CHROMIUM_PLATFORM_DIR = $${CHROMIUM_OUTPUT_DIR}/Debug
} else {
    CHROMIUM_PLATFORM_DIR = $${CHROMIUM_OUTPUT_DIR}/Release
}

QMAKE_CXXFLAGS += -fno-rtti
INCLUDEPATH = $${_PRO_FILE_PWD_}/$${DEPTH}
LIBS += -L$${CHROMIUM_PLATFORM_DIR}/lib.target -loxide-qt

OTHER_FILES = qmldir

qmldir.files = qmldir
unix {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}
