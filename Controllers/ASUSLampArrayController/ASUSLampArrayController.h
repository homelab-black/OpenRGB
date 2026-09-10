/*---------------------------------------------------------*\
| ASUSLampArrayController.h                                 |
|                                                           |
|   Driver for ASUS Vivobook LampArray Keyboard (ITE5570)   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "hidapi_wrapper.h"
#include <string>

class ASUSLampArrayController
{
public:
    ASUSLampArrayController(hid_device* dev_handle, const char* path, unsigned short pid);
    ~ASUSLampArrayController();

    std::string GetDeviceLocation();
    std::string GetSerialString();

    void SetHostMode(bool host_mode);
    void SetHardwareRainbow(bool enable);
    void SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char brightness);

private:
    hid_device* dev;
    std::string location;

    unsigned short product_id;
    unsigned char  firmware_report_id;
    unsigned char  color_report_id;
};
