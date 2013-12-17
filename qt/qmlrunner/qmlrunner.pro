TEMPLATE = app
TARGET = oxideqmlscene
QT += core core-private gui gui-private qml quick quick-private
CONFIG += qt

include($${OXIDE_QMAKE_PATH}/oxide_variables.pri)

DESTDIR = $${OXIDE_BUILD_OUT}/bin

SOURCES += main.cc
