TEMPLATE = aux
TARGET = gyp

DEPTH = ..

GYP_GENERATED_MAKEFILE = Makefile.oxide
GYP_MAKE_INVOKE = CFLAGS= CXXFLAGS= LDFLAGS= CPPFLAGS= make -C $$DEPTH -f $$GYP_GENERATED_MAKEFILE oxide

gyp_generate.target = $${DEPTH}/$${GYP_GENERATED_MAKEFILE}
gyp_generate.commands = $${DEPTH}/gyp_oxide -I$${PWD}/oxide_qt.gypi
QMAKE_EXTRA_TARGETS += gyp_generate

gypimpl.target = gypimpl
gypimpl.depends = gyp_generate
equals(OXIDE_DEBUG, "1") {
    gypimpl.commands = $$GYP_MAKE_INVOKE V=1
} else {
    gypimpl.commands = $$GYP_MAKE_INVOKE BUILDTYPE=Release
}
QMAKE_EXTRA_TARGETS += gypimpl
PRE_TARGETDEPS += gypimpl

OTHER_FILES += \
    oxide-qt/core/core.gyp \
    oxide-qt/oxide_qt.gypi \
    oxide-qt/qmlplugin/qmlplugin.gyp \
    oxide-qt/system.gyp

QMAKE_CLEAN += -r \
    $${DEPTH}/Makefile.oxide \
    `find $$DEPTH -name \"*.target.oxide.mk\"` \
    $${DEPTH}/chromium/src/out/
