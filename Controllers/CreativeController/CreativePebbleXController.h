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

    void                    UpdateDeviceLEDs(std::vector<RGBColor> led_colors, unsigned char data_packet_length, unsigned char mode_specific_led_num);
    void                    SetBrightness(unsigned char brightness);
    void                    SetSpeed(unsigned char brightness);
    void                    SetDirection(unsigned char brightness);
    void                    UpdateDeviceMode(unsigned char mode);

    std::string             GetDeviceLocation();
    std::string             GetSerialString();

private:
    hid_device*             dev;
    std::string             location;
};
