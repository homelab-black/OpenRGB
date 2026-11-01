
/*---------------------------------------------------------*\
|   ThermaltakeArgentMP1Controller.h                        |
|                                                           |
|   Controller class for Thermaltake Argent MP1 Mousepad    |
|                                                           |
|   John Scripter                               26 Oct 2026 |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include "hidapi/hidapi.h"
#include "RGBController.h"

class ThermaltakeArgentMP1Controller
{
public:
    ThermaltakeArgentMP1Controller(hid_device* dev_handle, const hid_device_info& info);
    ~ThermaltakeArgentMP1Controller();

    std::string GetDeviceLocation();
    std::string GetSerialString();

    void SendInitPacket();
    void SetLedColors(const std::vector<RGBColor>& colors);

private:
    hid_device* dev;
    std::string location;
    std::string serial;
};
