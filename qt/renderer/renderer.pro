TEMPLATE = aux

DEPTH = ../..

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

CHROMIUM_OUTPUT_DIR = $${DEPTH}/chromium/src/out
equals(OXIDE_DEBUG, "1") {
    CHROMIUM_PLATFORM_DIR = $${CHROMIUM_OUTPUT_DIR}/Debug
} else {
    CHROMIUM_PLATFORM_DIR = $${CHROMIUM_OUTPUT_DIR}/Release
}

renderer.path = $${PREFIX}/lib/oxide-qt/
renderer.files = $${CHROMIUM_PLATFORM_DIR}/oxide-renderer
renderer.CONFIG = no_check_exist executable
INSTALLS += renderer
