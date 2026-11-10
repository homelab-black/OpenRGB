/*---------------------------------------------------------*\
| DrevoKeyboardController.h                                 |
|                                                           |
|   Driver for Drevo keyboards                              |
|                                                           |
|   Dennis Blokland                             02 Jan 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <hidapi.h>
#include "RGBController.h"

#define DREVO_CONTROLLER_NAME                       "Drevo Keyboard Controller"

class DrevoKeyboardController
{
public:
    DrevoKeyboardController(hid_device* dev_handle, const char* path, std::string dev_name);
    ~DrevoKeyboardController();

    std::string             GetDeviceLocation();
    std::string             GetDeviceName();

    void                    SendDirect(const std::vector<RGBColor>& colors, const std::vector<led>& leds);

private:
    hid_device*             dev;
    std::string             location;
    std::string             name;
};
