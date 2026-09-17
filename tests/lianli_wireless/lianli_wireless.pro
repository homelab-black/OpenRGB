TEMPLATE = app
TARGET = lianli_protocol_test
CONFIG += console c++17
CONFIG -= app_bundle qt debug_and_release
DEFINES += _IS_USED_MULTITHREAD=0
CONTROLLER = $$PWD/../../Controllers/LianLiWirelessController
VENDOR = $$PWD/../../dependencies
INCLUDEPATH += $$CONTROLLER $$VENDOR $$VENDOR/tinyuz $$VENDOR/HDiffPatch
SOURCES += $$PWD/lianli_protocol_test.cpp \
    $$CONTROLLER/LianLiWirelessProtocol.cpp \
    $$CONTROLLER/LianLiWirelessCodec.cpp \
    $$CONTROLLER/LianLiWirelessRuntime.cpp \
    $$CONTROLLER/LianLiWirelessService.cpp \
    $$VENDOR/tinyuz/compress/tuz_enc.cpp \
    $$VENDOR/tinyuz/compress/tuz_enc_private/tuz_enc_clip.cpp \
    $$VENDOR/tinyuz/compress/tuz_enc_private/tuz_enc_code.cpp \
    $$VENDOR/tinyuz/compress/tuz_enc_private/tuz_enc_match.cpp \
    $$VENDOR/tinyuz/compress/tuz_enc_private/tuz_sstring.cpp \
    $$VENDOR/tinyuz/decompress/tuz_dec.c \
    $$VENDOR/HDiffPatch/libHDiffPatch/HDiff/private_diff/libdivsufsort/divsufsort.cpp
unix: LIBS += -lpthread
