TEMPLATE = app
TARGET = RedragonK652StartupSafety
CONFIG += console c++17 release
CONFIG -= app_bundle
QT -= core gui
DEFINES += HID_API_NO_EXPORT_DEFINE

INCLUDEPATH += $$PWD/stubs
INCLUDEPATH += $$PWD/../../dependencies/hidapi-hotplug-win/include
INCLUDEPATH += $$PWD/../../Controllers/SinowealthController/RedragonK652Controller
INCLUDEPATH += $$PWD/../../RGBController

SOURCES += $$PWD/RedragonK652StartupSafety.cpp
SOURCES += $$PWD/../../Controllers/SinowealthController/RedragonK652Controller/RedragonK652Controller.cpp
SOURCES += $$PWD/../../Controllers/SinowealthController/RedragonK652Controller/RedragonK652DeviceRegistry.cpp
SOURCES += $$PWD/../../Controllers/SinowealthController/RedragonK652Controller/RGBController_RedragonK652.cpp
SOURCES += $$PWD/../../RGBController/RGBControllerKeyNames.cpp

win32:LIBS += -lbcrypt
