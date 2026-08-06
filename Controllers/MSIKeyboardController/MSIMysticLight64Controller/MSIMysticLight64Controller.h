/*---------------------------------------------------------*\
| MSIMysticLight64Controller.h                              |
|                                                           |
|   Driver for MSI Mystic Light 64-byte motherboard         |
|                                                           |
|   T-bond                                      03 Apr 2020 |
|   Adam Honse                                  06 Mar 2021 |
|   Elchanan Haas                               23 Aug 2022 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <cstring>
#include <limits>
#include <hidapi.h>
#include "RGBController.h"

enum MSI_64_BRIGHTNESS
{
    MSI_64_BRIGHTNESS_OFF                           = 0,
    MSI_64_BRIGHTNESS_LEVEL_10                      = 1,
    MSI_64_BRIGHTNESS_LEVEL_20                      = 2,
    MSI_64_BRIGHTNESS_LEVEL_30                      = 3,
    MSI_64_BRIGHTNESS_LEVEL_40                      = 4,
    MSI_64_BRIGHTNESS_LEVEL_50                      = 5,
    MSI_64_BRIGHTNESS_LEVEL_60                      = 6,
    MSI_64_BRIGHTNESS_LEVEL_70                      = 7,
    MSI_64_BRIGHTNESS_LEVEL_80                      = 8,
    MSI_64_BRIGHTNESS_LEVEL_90                      = 9,
    MSI_64_BRIGHTNESS_LEVEL_100                     = 10,
};

enum MSI_64_MODE
{
    MSI_64_OFF                                      = 0,
    MSI_64_STEADY                                   = 1,
    MSI_64_BREATHING                                = 2,
    MSI_64_PULSE                                    = 3,
    MSI_64_DOUBLE_PULSE                             = 4,
    MSI_64_CYCLE                                    = 5,
    MSI_64_SMOOTH_CYCLE                             = 6,
};

enum MSI_64_SPEED
{
    MSI_64_SPEED_LOW                                = 0,
    MSI_64_SPEED_MEDIUM                             = 1,
    MSI_64_SPEED_HIGH                               = 2,
};

class MSIMysticLight64Controller
{
public:
    MSIMysticLight64Controller
        (
        hid_device*     handle,
        const char      *path
        );
    ~MSIMysticLight64Controller();

    void            SetMode
                        (
                        MSI_64_MODE        mode,
                        MSI_64_SPEED       speed,
                        MSI_64_BRIGHTNESS  brightness,
                        unsigned int num_colors,
                        Color colors[]
                        );

    std::string     GetDeviceName();
    std::string     GetDeviceLocation();
    std::string     GetFWVersion();
    std::string     GetSerial();

private:
    hid_device*                     dev;
    std::string                     location;
};
