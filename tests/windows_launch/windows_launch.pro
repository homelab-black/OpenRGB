TEMPLATE = app
TARGET = windows_launch_policy_test
CONFIG += console c++17
CONFIG -= app_bundle qt debug_and_release
!win32: error(This test requires Windows.)
SOURCES += $$PWD/windows_launch_policy_test.cpp
