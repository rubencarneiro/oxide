TEMPLATE = aux
TARGET = gyp

DEPTH = ..

GYP_GENERATED_MAKEFILE = Makefile.oxide
GYP_MAKE_INVOKE = CFLAGS= CXXFLAGS= LDFLAGS= CPPFLAGS= make -C $$DEPTH -f $$GYP_GENERATED_MAKEFILE oxide

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

gyp_generate.target = $${DEPTH}/$${GYP_GENERATED_MAKEFILE}
gyp_generate.commands = \
    cd $$DEPTH ; ./gyp_oxide \
    -I$${PWD}/qt.gypi \
    -Dlinux_sandbox_path=\'$${PREFIX}/lib/oxide-qt/oxide-sandbox\'
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
    qt/lib/lib.gyp \
    qt/qmlplugin/qmlplugin.gyp \
    qt/qt.gypi \
    qt/renderer/renderer.gyp \
    qt/system.gyp

QMAKE_CLEAN += -r \
    $${DEPTH}/Makefile.oxide \
    `find $$DEPTH -name \"*.target.oxide.mk\"` \
    $${DEPTH}/chromium/src/out/
