/*---------------------------------------------------------*\
| LogitechG110Controller.h                                  |
|                                                           |
|   Driver for Logitech G110 Gaming Keyboard                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "RGBController.h"

enum
{
    LOGITECH_G110_MODE_STATIC           = 0x01,
};

class LogitechG110Controller
{
public:
    LogitechG110Controller(hid_device* dev_handle, const char* path);
    ~LogitechG110Controller();

    std::string GetDeviceLocation();
    std::string GetSerialString();

    void        SetColor(unsigned char red, unsigned char blue, unsigned char brightness);

private:
    hid_device* dev;
    std::string location;
};
