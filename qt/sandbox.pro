TEMPLATE = aux

DEPTH = ..

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

CHROMIUM_OUTPUT_DIR = $${DEPTH}/chromium/src/out
equals(OXIDE_DEBUG, "1") {
    CHROMIUM_PLATFORM_DIR = $${CHROMIUM_OUTPUT_DIR}/Debug
} else {
    CHROMIUM_PLATFORM_DIR = $${CHROMIUM_OUTPUT_DIR}/Release
}

sandbox.path = $${PREFIX}/lib/oxide-qt/
sandbox.files = $${CHROMIUM_PLATFORM_DIR}/chrome-sandbox
sandbox.extra = cp $${CHROMIUM_PLATFORM_DIR}/chrome_sandbox $$sandbox.files
sandbox.CONFIG = no_check_exist executable
INSTALLS += sandbox

setsuidbit.path = $$sandbox.path
setsuidbit.commands = chmod u+s $${sandbox.path}/chrome-sandbox
setsuidbit.depends = sandbox
INSTALLS += setsuidbit
