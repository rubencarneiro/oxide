TEMPLATE = subdirs
CONFIG += ordered

equals(OXIDE_DEBUG, "1") {
    CONFIG += debug
}

oxidegyp.file = oxide-qt/gyp.pro
SUBDIRS += oxidegyp

oxideprivate.file = oxide-qt/core/oxideprivate.pro
SUBDIRS += oxideprivate

qmlplugin.file = oxide-qt/qmlplugin/qmlplugin.pro
SUBDIRS += qmlplugin

renderer.file = oxide-qt/renderer/renderer.pro
SUBDIRS += renderer

sandbox.file = oxide-qt/sandbox.pro
SUBDIRS += sandbox

OTHER_FILES += \
    gyp_oxide \
    oxide.gyp \
    oxide/oxide.gypi \
    oxide/oxide_common.gyp
