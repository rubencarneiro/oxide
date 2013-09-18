TEMPLATE = app
TARGET = tst_qmltests
CONFIG += warn_on oxideqmltestcase
SOURCES += tst_qmltests.cc

include($${OXIDE_QMAKE_PATH}/oxide_variables.pri)

IMPORTPATH = $${OUT_PWD}/imports
QMLSOURCEPATH = $${_PRO_FILE_PWD_}/qml
