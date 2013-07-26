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

lib.path = $${PREFIX}/lib
lib.files = $${CHROMIUM_PLATFORM_DIR}/lib.target/$${QMAKE_PREFIX_SHLIB}oxide-qt.$${QMAKE_EXTENSION_SHLIB}* \
lib.CONFIG = no_check_exist
INSTALLS += lib

extras.path = $${PREFIX}/lib/oxide-qt
extras.files = \
    $${CHROMIUM_PLATFORM_DIR}/obj/gen/repack/oxide.pak \
    $${CHROMIUM_PLATFORM_DIR}/obj/gen/repack/oxide_100_percent.pak
extras.CONFIG = no_check_exist
INSTALLS += extras
