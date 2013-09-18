TEMPLATE = aux
TARGET = dummy

include($${OXIDE_QMAKE_PATH}/oxide_variables.pri)

RESOURCE_DESTDIR = $${PREFIX}/lib/oxide-qt/

lib.path = $${PREFIX}/lib
lib.files = $${CHROMIUM_LIB_DIR}/liboxide-qt.so* \
lib.CONFIG = no_check_exist
INSTALLS += lib

renderer.path = $$RESOURCE_DESTDIR
renderer.files = $${CHROMIUM_PLAT_DIR}/oxide-renderer
renderer.CONFIG = no_check_exist executable
INSTALLS += renderer

sandbox.path = $$RESOURCE_DESTDIR
sandbox.files = $${CHROMIUM_PLAT_DIR}/chrome-sandbox
sandbox.extra = cp $${CHROMIUM_PLAT_DIR}/chrome_sandbox $$sandbox.files
sandbox.CONFIG = no_check_exist executable
INSTALLS += sandbox

setsuidbit.path = $$sandbox.path
setsuidbit.commands = chmod u+s $${sandbox.path}/chrome-sandbox
setsuidbit.depends = $$sandbox.files
INSTALLS += setsuidbit

extras.path = $$RESOURCE_DESTDIR
extras.files = \
    $${CHROMIUM_PLAT_DIR}/oxide.pak \
    $${CHROMIUM_PLAT_DIR}/oxide_100_percent.pak
extras.CONFIG = no_check_exist
INSTALLS += extras
