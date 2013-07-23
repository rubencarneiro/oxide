TEMPLATE = aux

DEPTH = ../..

isEmpty(PREFIX) {
    PREFIX = /usr/local
}
isEmpty(QMAKE_EXTENSION_SHLIB) {
    QMAKE_EXTENSION_SHLIB = so
}

CHROMIUM_OUTPUT_DIR = $${DEPTH}/chromium/src/out
equals(OXIDE_DEBUG, "1") {
    CHROMIUM_PLATFORM_DIR = $${CHROMIUM_OUTPUT_DIR}/Debug
} else {
    CHROMIUM_PLATFORM_DIR = $${CHROMIUM_OUTPUT_DIR}/Release
}

oxideprivate.path = $${PREFIX}/lib/oxide-qt/
oxideprivate.files = $${CHROMIUM_PLATFORM_DIR}/lib.target/$${QMAKE_PREFIX_SHLIB}oxideprivate.$${QMAKE_EXTENSION_SHLIB}
oxideprivate.CONFIG = no_check_exist
INSTALLS += oxideprivate
