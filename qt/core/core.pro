CONFIG += gyp disable_check
TARGET = oxide-qt
GYP_TYPE = lib

include($${OXIDE_QMAKE_PATH}/oxide_variables.pri)

GYP_LIBVERSION = $$OXIDE_QT_LIBVERSION

resources.path = $$LIBEXECDIR
resources.files = \
    $${CHROMIUM_OUT_PLAT_DIR}/oxide.pak \
    $${CHROMIUM_OUT_PLAT_DIR}/oxide_100_percent.pak
resources.CONFIG = no_check_exist
INSTALLS += resources
