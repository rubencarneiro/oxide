TEMPLATE = lib
TARGET = qmloxideplugin
QT += qml quick
CONFIG += qt plugin
DEPTH = ../..

TARGET = $$qtLibraryTarget($$TARGET)
uri = com.canonical.Oxide

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

INCLUDEPATH += \
    $$DEPTH \
    $${DEPTH}/oxide-qt \
    $${DEPTH}/chromium/src

equals(OXIDE_DEBUG, "1") {
    LIBS += -L$${DEPTH}/chromium/src/out/Debug/lib.target -loxideprivate
} else {
    LIBS += -L$${DEPTH}/chromium/src/out/Release/lib.target -loxideprivate
}

# Input
SOURCES += \
    oxide_qml_plugin.cpp \
    oxide_qquick_web_view.cpp \
    oxide_qquick_web_view_context.cpp

HEADERS += \
    oxide_qquick_web_view.h \
    oxide_qquick_web_view_context.h

OTHER_FILES = qmldir

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files = qmldir
unix {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
    QMAKE_LFLAGS += '-Wl,-rpath,\'\$\$ORIGIN/$$system(python -c \'import os.path; print os.path.relpath(\"$${PREFIX}/lib/oxide-qt\", \"$$installPath\")\')\'' 
}
