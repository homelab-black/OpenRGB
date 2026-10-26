/*---------------------------------------------------------*\
| AMDRadeonRX7900XTXController.h                            |
|                                                           |
|   AMD Radeon RX 7900 XTX reference RGB over SMU I2C       |
|                                                           |
|   Tanner Krewson (tannerkrewson)               25 Oct 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <mutex>
#include <string>
#include "RGBControllerInterface.h"
#include "i2c_smbus.h"

#define AMD_RX7900XTX_RGB_ADDR           0x5A
#define AMD_RX7900XTX_MODE_STATIC        0x00
#define AMD_RX7900XTX_MODE_BREATHING     0x01
#define AMD_RX7900XTX_MODE_CYCLE         0x02
#define AMD_RX7900XTX_MODE_OFF           0xFE

class AMDRadeonRX7900XTXController
{
public:
    AMDRadeonRX7900XTXController(i2c_smbus_interface* bus);

    static bool IsSupportedBus(const i2c_smbus_interface* bus);
    bool        Detect();
    bool        SetMode(unsigned char mode, unsigned char brightness, RGBColor color);
    std::string GetLocation();

private:
    bool        Transfer(const unsigned char* request, unsigned char* response = nullptr);
    bool        Apply();
    bool        UpdateMode(unsigned char mode, unsigned char brightness, RGBColor color);

    i2c_smbus_interface* bus;
    std::mutex          mutex;
    bool                detected;
    bool                initialized;
};
