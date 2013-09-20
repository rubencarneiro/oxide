CONFIG += gyp
TARGET = chrome_sandbox
GYP_TYPE = libexec

TARGET.bin_name = chrome-sandbox
TARGET.post_install = chmod u+s
