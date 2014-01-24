TEMPLATE = subdirs

include($${OXIDE_QMAKE_PATH}/oxide_variables.pri)

MESSAGES = obj/gen/oxide/shared/common/oxide_i18n_messages.h

potfile.target = oxide.pot
potfile.depends = $${CHROMIUM_OUT_PLAT_DIR}/$${MESSAGES}
potfile.commands = xgettext -o $$potfile.target -D $${CHROMIUM_OUT_PLAT_DIR} --from-code=UTF-8 --c++ --add-comments=TRANSLATORS --package-name=oxide $${MESSAGES}
QMAKE_EXTRA_TARGETS += potfile
