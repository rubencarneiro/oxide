TEMPLATE = subdirs

equals(OXIDE_DEBUG, "1") {
    CONFIG += debug
}

oxideqtprivate.file = oxide-qt/core/oxideprivate.pro
SUBDIRS += oxideqtprivate

qmlplugin.file = oxide-qt/qmlplugin/qmlplugin.pro
SUBDIRS += qmlplugin

OTHER_FILES += \
    gyp_oxide \
    oxide.gyp \
    oxide/oxide.gypi \
    oxide/oxide_common.gyp
