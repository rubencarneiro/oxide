TEMPLATE = subdirs
CONFIG += ordered

core.file = qt/core/core.pro
SUBDIRS += core

renderer.file = qt/renderer/renderer.pro
SUBDIRS += renderer

sandbox.file = qt/sandbox/sandbox.pro
SUBDIRS += sandbox

qmlplugin.file = qt/qmlplugin/qmlplugin.pro
SUBDIRS += qmlplugin

testutils.file = qt/tests/utils/testutils.pro
SUBDIRS += testutils

qmltests.file = qt/tests/qmltests/qmltests.pro
SUBDIRS += qmltests

QMAKE_CLEAN += -r \
    $${OXIDE_SRC_ROOT}/Makefile.oxide \
    `find $$OXIDE_SRC_ROOT -name \"*.target.oxide.mk\"` \
    $$CHROMIUM_OUT_DIR
