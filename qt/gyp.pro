TEMPLATE = aux
TARGET = gyp

DEPTH = ..

GYP_GENERATED_MAKEFILE = Makefile.oxide
GYP_MAKE_INVOKE = CFLAGS= CXXFLAGS= LDFLAGS= CPPFLAGS= make -C $$DEPTH -f $$GYP_GENERATED_MAKEFILE oxide

isEmpty(PREFIX) {
    PREFIX = /usr/local
}
isEmpty(QMAKE_EXTENSION_SHLIB) {
    QMAKE_EXTENSION_SHLIB = so
}

gyp_generate.target = $${DEPTH}/$${GYP_GENERATED_MAKEFILE}
gyp_generate.commands = \
    cd $$DEPTH ; ./gyp_oxide \
    -I$${PWD}/qt.gypi
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

CHROMIUM_OUTPUT_DIR = $${DEPTH}/chromium/src/out
equals(OXIDE_DEBUG, "1") {
    CHROMIUM_PLATFORM_DIR = $${CHROMIUM_OUTPUT_DIR}/Debug
} else {
    CHROMIUM_PLATFORM_DIR = $${CHROMIUM_OUTPUT_DIR}/Release
}

gyppost.target = gyppost
gyppost.commands = \
    cd $${CHROMIUM_PLATFORM_DIR}/lib.target && \
    ln -f -s $${QMAKE_PREFIX_SHLIB}oxide-qt.$${QMAKE_EXTENSION_SHLIB}.0 $${QMAKE_PREFIX_SHLIB}oxide-qt.$${QMAKE_EXTENSION_SHLIB}
QMAKE_EXTRA_TARGETS += gyppost
POST_TARGETDEPS += gyppost

OTHER_FILES += \
    gyp_oxide \
    oxide.gyp \
    qt/lib/lib.gyp \
    qt/qmlplugin/qmlplugin.gyp \
    qt/qt.gypi \
    qt/renderer/renderer.gyp \
    qt/system.gyp \
    shared/shared.gypi \
    shared/shared.gyp

QMAKE_CLEAN += -r \
    $${DEPTH}/Makefile.oxide \
    `find $$DEPTH -name \"*.target.oxide.mk\"` \
    $${DEPTH}/chromium/src/out/
