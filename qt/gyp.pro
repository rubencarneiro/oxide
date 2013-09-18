TEMPLATE = aux
TARGET = gyp

include($${OXIDE_QMAKE_PATH}/oxide_variables.pri)

GYP_GENERATED_MAKEFILE = Makefile.oxide
GYP_MAKE_INVOKE = CFLAGS= CXXFLAGS= LDFLAGS= CPPFLAGS= make -C $$OXIDE_SRC_ROOT -f $$GYP_GENERATED_MAKEFILE oxide

gyp_generate.target = $${OXIDE_SRC_ROOT}/$${GYP_GENERATED_MAKEFILE}
gyp_generate.commands = \
    cd $$OXIDE_SRC_ROOT ; ./gyp_oxide \
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

gyppost.target = gyppost
gyppost.commands = \
    cd $$CHROMIUM_LIB_DIR && \
    ln -f -s liboxide-qt.so.0 liboxide-qt.so
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
    $${OXIDE_SRC_ROOT}/Makefile.oxide \
    `find $$OXIDE_SRC_ROOT -name \"*.target.oxide.mk\"` \
    $$CHROMIUM_OUT_DIR
