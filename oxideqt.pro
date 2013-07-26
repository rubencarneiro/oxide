TEMPLATE = subdirs
CONFIG += ordered

equals(OXIDE_DEBUG, "1") {
    CONFIG += debug
}

oxidegyp.file = qt/gyp.pro
SUBDIRS += oxidegyp

oxidelib.file = qt/lib/lib.pro
SUBDIRS += oxidelib

qmlplugin.file = qt/qmlplugin/qmlplugin.pro
SUBDIRS += qmlplugin

renderer.file = qt/renderer/renderer.pro
SUBDIRS += renderer

sandbox.file = qt/sandbox.pro
SUBDIRS += sandbox

OTHER_FILES += \
    gyp_oxide \
    oxide.gyp \
    oxide.gypi \
    shared/shared.gyp
