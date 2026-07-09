/*---------------------------------------------------------*\
| SkydimoHIDController.h                                    |
|                                                           |
|   Driver for Skydimo HID devices                          |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <hidapi.h>
#include "RGBControllerInterface.h"

class SkydimoHIDController
{
public:
    SkydimoHIDController(hid_device* dev_handle, const char* path);
    ~SkydimoHIDController();

    std::string             GetDeviceName();
    std::string             GetSerial();
    std::string             GetLocation();

    void                    SetLEDs(const std::vector<RGBColor>& colors);

private:
    hid_device*             dev;
    std::string             device_name;
    std::string             serial;
    std::string             location;

    void                    SendPacket(unsigned char start_led, const unsigned char* chunk, size_t chunk_len);
};
