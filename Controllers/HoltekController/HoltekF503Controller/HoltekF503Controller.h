/*---------------------------------------------------------*\
| HoltekF503Controller.h                                    |
|                                                           |
|   Driver for Holtek mouse                                 |
|                                                           |
|   Remigiusz Pietroń (remememe)                24 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>

enum
{
    HOLTEK_F503_MODE_STATIC    = 0x01,
    HOLTEK_F503_MODE_BREATHING = 0x01,
    HOLTEK_F503_MODE_WAVE      = 0x02,
};

enum
{
    HOLTEK_F503_SPEED_SLOW    = 0x08,   /* Slow speed         */
    HOLTEK_F503_SPEED_NORMAL  = 0x05,   /* Normal speed       */
    HOLTEK_F503_SPEED_FAST    = 0x01,   /* Fast speed         */
};

enum
{
    HOLTEK_F503_DIRECTION_LEFT  = 0x01,
    HOLTEK_F503_DIRECTION_RIGHT = 0x00,
};
class HoltekF503Controller
{
public:
    HoltekF503Controller(hid_device* dev_handle, const char* path, std::string dev_name);
    ~HoltekF503Controller();

    std::string GetDeviceLocation();
    std::string GetNameString();
    std::string GetSerialString();

    void SendCustomColor(unsigned char red, unsigned char green, unsigned char blue,unsigned char mode,unsigned char speed, unsigned char flag);
    void SendBrightness(unsigned char Brightness);
    void Apply();

private:
    hid_device*             dev;
    std::string             location;
    std::string             name;
};
