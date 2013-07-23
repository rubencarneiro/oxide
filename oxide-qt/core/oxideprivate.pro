TEMPLATE = aux
TARGET = oxideprivate

DEPTH = ../..

GYP_GENERATED_MAKEFILE = Makefile.oxideprivate
GYP_MAKE_INVOKE = CFLAGS= CXXFLAGS= LDFLAGS= CPPFLAGS= make -C $$DEPTH -f $$GYP_GENERATED_MAKEFILE oxideprivate

isEmpty(PREFIX) {
    PREFIX = /usr/local
}
isEmpty(QMAKE_EXTENSION_SHLIB) {
    QMAKE_EXTENSION_SHLIB=so
}

oxidegyp.target = $${DEPTH}/$${GYP_GENERATED_MAKEFILE}
oxidegyp.commands = cd $${DEPTH} && ./gyp_oxide -I$${PWD}/oxide_qt.gypi
QMAKE_EXTRA_TARGETS += oxidegyp

oxideprivateimpl.target = oxideprivateimpl
oxideprivateimpl.depends = oxidegyp
equals(OXIDE_DEBUG, "1") {
    oxideprivateimpl.commands = $$GYP_MAKE_INVOKE V=1
    oxideprivateinstall.files = $${DEPTH}/chromium/src/out/Debug/lib.target/$${QMAKE_PREFIX_SHLIB}oxideprivate.$${QMAKE_EXTENSION_SHLIB}
} else {
    oxideprivateimpl.commands = $$GYP_MAKE_INVOKE BUILDTYPE=Release
    oxideprivateinstall.files = $${DEPTH}/chromium/src/out/Release/lib.target/$${QMAKE_PREFIX_SHLIB}oxideprivate.$${QMAKE_EXTENSION_SHLIB}
}
QMAKE_EXTRA_TARGETS += oxideprivateimpl
PRE_TARGETDEPS += oxideprivateimpl

oxideprivateinstall.path = $${PREFIX}/lib/oxide-qt/
oxideprivateinstall.CONFIG += no_check_exist
INSTALLS += oxideprivateinstall

OTHER_FILES += \
    oxide-qt/core/oxide_qt.gyp \
    oxide-qt/core/oxide_qt.gypi

QMAKE_CLEAN += -r \
    $${DEPTH}/Makefile.oxideprivate \
    `find $$DEPTH -name \"*.target.oxideprivate.mk\"` \
    $${DEPTH}/chromium/src/out/
