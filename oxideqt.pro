TEMPLATE = subdirs
CONFIG += ordered

include($${OXIDE_QMAKE_PATH}/oxide_variables.pri)

core.file = qt/core/core.pro
SUBDIRS += core

renderer.file = qt/renderer/renderer.pro
SUBDIRS += renderer

sandbox.file = qt/sandbox/sandbox.pro
SUBDIRS += sandbox

qmlplugin.file = qt/quick/qmlplugin.pro
SUBDIRS += qmlplugin

qmlrunner.file = qt/qmlrunner/qmlrunner.pro
SUBDIRS += qmlrunner

testutils.file = qt/tests/utils/testutils.pro
SUBDIRS += testutils

qmltests.file = qt/tests/qmltests/qmltests.pro
SUBDIRS += qmltests

oxideclean.commands = \
    $(DEL_FILE) $${OXIDE_SRC_ROOT}/Makefile.oxide && \
    $(DEL_FILE) `find $$OXIDE_SRC_ROOT -name \"*.target.oxide.mk\"` && \
    $(DEL_FILE) `find $$OXIDE_SRC_ROOT -name \"*.host.oxide.mk\"` && \
    rm -rf $$CHROMIUM_OUT_DIR

CLEAN_DEPS += oxideclean
QMAKE_EXTRA_TARGETS += oxideclean
