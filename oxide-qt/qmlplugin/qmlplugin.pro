TEMPLATE = aux
uri = com.canonical.Oxide

DEPTH = ../..

isEmpty(QMAKE_EXTENSION_SHLIB) {
    QMAKE_EXTENSION_SHLIB = so
}

CHROMIUM_OUTPUT_DIR = $${DEPTH}/chromium/src/out
equals(OXIDE_DEBUG, "1") {
    CHROMIUM_PLATFORM_DIR = $${CHROMIUM_OUTPUT_DIR}/Debug
} else {
    CHROMIUM_PLATFORM_DIR = $${CHROMIUM_OUTPUT_DIR}/Release
}

installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
qmldir.files = qmldir
qmldir.path = $$installPath
qmlplugin.files = $${CHROMIUM_PLATFORM_DIR}/lib.target/$${QMAKE_PREFIX_SHLIB}qmloxideplugin.$${QMAKE_EXTENSION_SHLIB}
qmlplugin.path = $$installPath
qmlplugin.CONFIG = no_check_exist

INSTALLS += qmlplugin qmldir

OTHER_FILES = qmldir
