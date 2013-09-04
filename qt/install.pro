TEMPLATE = aux
TARGET = dummy

DEPTH = ..

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

RESOURCE_DESTDIR = $${PREFIX}/lib/oxide-qt/

lib.path = $${PREFIX}/lib
lib.files = $${CHROMIUM_PLATFORM_DIR}/lib.target/$${QMAKE_PREFIX_SHLIB}oxide-qt.$${QMAKE_EXTENSION_SHLIB}* \
lib.CONFIG = no_check_exist
INSTALLS += lib

renderer.path = $$RESOURCE_DESTDIR
renderer.files = $${CHROMIUM_PLATFORM_DIR}/oxide-renderer
renderer.CONFIG = no_check_exist executable
INSTALLS += renderer

sandbox.path = $$RESOURCE_DESTDIR
sandbox.files = $${CHROMIUM_PLATFORM_DIR}/chrome-sandbox
sandbox.extra = cp $${CHROMIUM_PLATFORM_DIR}/chrome_sandbox $$sandbox.files
sandbox.CONFIG = no_check_exist executable
INSTALLS += sandbox

setsuidbit.path = $$sandbox.path
setsuidbit.commands = chmod u+s $${sandbox.path}/chrome-sandbox
setsuidbit.depends = $$sandbox.files
INSTALLS += setsuidbit

extras.path = $$RESOURCE_DESTDIR
extras.files = \
    $${CHROMIUM_PLATFORM_DIR}/oxide.pak \
    $${CHROMIUM_PLATFORM_DIR}/oxide_100_percent.pak
extras.CONFIG = no_check_exist
INSTALLS += extras
