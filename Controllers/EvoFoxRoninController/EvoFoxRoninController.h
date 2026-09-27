/*---------------------------------------------------------*\
| EvoFoxRoninController.h                                   |
|                                                           |
|   Driver for EvoFox Ronin TKL RGB keyboard                |
|                                                           |
|   Protocol reverse engineered from USB HID traffic.       |
|                                                           |
|   <Atif Usmani> (<AtifUsmani>)               <30 09 2026> |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <cstddef>
#include <string>
#include <hidapi.h>
#include "RGBController.h"

#define EVOFOX_RONIN_VID 0x320F
#define EVOFOX_RONIN_PID 0x5055
#define EVOFOX_RONIN_INTERFACE 1

/*-----------------------------------------------------------*\
| Usage page and report size confirmed against real hardware: |
| hid.enumerate() on the physical device reports interface 1  |
| exposing a vendor-defined collection at usage_page=0xFF1C,  |
| usage=0x92, and 91-byte reports were exercised successfully |
| end-to-end (effect + direction changes all behaved          |
| correctly) during testing.                                   |
\*-----------------------------------------------------------*/
#define EVOFOX_RONIN_USAGE_PAGE 0xFF1C
#define EVOFOX_RONIN_REPORT_SIZE 91

enum
{
    EVOFOX_RONIN_EFFECT_CORRUGATED = 0x01,
    EVOFOX_RONIN_EFFECT_CLOUD      = 0x02,
    EVOFOX_RONIN_EFFECT_SERPENTINE = 0x03,
    EVOFOX_RONIN_EFFECT_SPECTRUM   = 0x04,
    EVOFOX_RONIN_EFFECT_BREATH     = 0x05,
    EVOFOX_RONIN_EFFECT_STATIC     = 0x06,
    EVOFOX_RONIN_EFFECT_STARS      = 0x0A,
    EVOFOX_RONIN_EFFECT_FLOWERS    = 0x0B,
    EVOFOX_RONIN_EFFECT_ROLL       = 0x0C,
    EVOFOX_RONIN_EFFECT_WAVE       = 0x0D,
    EVOFOX_RONIN_EFFECT_WAVE_ALT   = 0x0E,
    EVOFOX_RONIN_EFFECT_RAIN       = 0x0F,
    EVOFOX_RONIN_EFFECT_SCAN       = 0x10,
};

class EvoFoxRoninController
{
public:
    EvoFoxRoninController(hid_device* dev_handle, const char* path, std::string dev_name);
    ~EvoFoxRoninController();

    std::string GetDeviceLocation();
    std::string GetDeviceName();
    std::string GetSerialString();

    void SetLightingConfig(const mode &m, RGBColor color);

private:
    hid_device* dev;
    std::string location;
    std::string name;

    void SendReport(const unsigned char* data, size_t length);
    void SendTransaction(const unsigned char* command, size_t command_length);
};
