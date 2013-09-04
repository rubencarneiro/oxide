TEMPLATE = subdirs
CONFIG += ordered

oxidegyp.file = qt/gyp.pro
SUBDIRS += oxidegyp

qmlplugin.file = qt/qmlplugin/qmlplugin.pro
SUBDIRS += qmlplugin

install.file = qt/install.pro
SUBDIRS += install
