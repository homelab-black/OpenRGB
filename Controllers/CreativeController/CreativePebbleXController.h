/*---------------------------------------------------------*\
| CreativePebbleXController.h                               |
|                                                           |
|   Driver for Creative Pebble X                            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <cstring>
#include <hidapi.h>
#include "RGBController.h"

class CreativePebbleXController
{
public:
    CreativePebbleXController(hid_device* dev_handle, const char* path);
    ~CreativePebbleXController();

    void                    UpdateDeviceLEDs(const std::vector<RGBColor>& led_colors, unsigned char brightness);
    void                    SetBrightness(unsigned char brightness);
    void                    UpdateDeviceMode(unsigned char mode);
    std::string             GetDeviceLocation();
    std::string             GetSerialString();

private:
    hid_device*             dev;
    std::string             location;

    void            SendBrightness
        (
            unsigned char   brightness
            );

};
