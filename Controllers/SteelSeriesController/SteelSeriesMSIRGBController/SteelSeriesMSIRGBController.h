/*---------------------------------------------------------*\
| SteelSeriesMSIRGBController.h                             |
|                                                           |
|   Driver for MSI laptop SteelSeries KLC/ALC RGB devices   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <hidapi.h>
#include "RGBController.h"

typedef enum
{
    STEELSERIES_MSI_RGB_KLC,
    STEELSERIES_MSI_RGB_ALC,

} steelseries_msi_rgb_device;

class SteelSeriesMSIRGBController
{
public:
    SteelSeriesMSIRGBController(hid_device* dev_handle, const char* path, std::string dev_name, steelseries_msi_rgb_device device_type);
    ~SteelSeriesMSIRGBController();

    std::string                 GetDeviceLocation();
    std::string                 GetDeviceName();
    std::string                 GetSerialString();
    steelseries_msi_rgb_device  GetDeviceType();

    void                        SetLEDs(std::vector<led> leds, std::vector<RGBColor> colors);

private:
    hid_device*                 dev;
    std::string                 location;
    std::string                 name;
    steelseries_msi_rgb_device  type;
};
