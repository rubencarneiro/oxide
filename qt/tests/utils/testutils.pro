TEMPLATE = lib
TARGET = qmloxidetestingplugin
QT += qml
CONFIG += qt plugin

include($${OXIDE_QMAKE_PATH}/oxide_variables.pri)

uri = com.canonical.Oxide.Testing
TARGET.module_name = $$replace(uri, \\., /)

DESTDIR = $${OXIDE_BUILD_OUT}/imports/$${TARGET.module_name}

SOURCES += oxide_qml_testing_plugin.cc

OTHER_FILES = qmldir TestWebView.qml TestUtils.js TestUtilsSlave.js

for (file, OTHER_FILES) {
    var = copy2build_$${file}
    eval($${var}.target = $${OXIDE_BUILD_OUT}/imports/$${TARGET.module_name}/$${file})
    eval($${var}.depends = $${_PRO_FILE_PWD_}/$${file})
    eval($${var}.commands = $${QMAKE_COPY} $${_PRO_FILE_PWD_}/$${file} $$eval($${var}.target))
    eval(QMAKE_EXTRA_TARGETS += $${var})
    eval(POST_TARGETDEPS += $$eval($${var}.target))
}
