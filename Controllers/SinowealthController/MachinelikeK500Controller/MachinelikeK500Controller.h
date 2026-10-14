/*---------------------------------------------------------*\
| MachinelikeK500Controller.h                               |
|                                                           |
|   This is a version of SinowealthKeyboardController       |
|   Modified to support Machinelike K500 keyboards          |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include <vector>
#include <hidapi.h>

class MachinelikeK500Controller
{
public:
    MachinelikeK500Controller(hid_device* dev_cmd_handle, hid_device* dev_data_handle, const char *_path, const std::string& name); //RGB, Command, path
    ~MachinelikeK500Controller();

    unsigned int    GetLEDCount();
    std::string     GetLocation();
    std::string     GetName();
    std::string     GetSerialString();
    unsigned char   GetCurrentMode();

    void            SetLEDColor(RGBColor* color_buf);
    void            SetStaticColor(RGBColor* color_buf);
    void            SetMode(unsigned char mode, unsigned char brightness, unsigned char speed, unsigned char color_mode);
    void            GetProfile();
    void            ReadFirmwareInfo();
    void            SetLEDsDirect(std::vector<RGBColor> colors);
private:
    hid_device*     dev_cmd;
    hid_device*     dev_data;
    device_type     type;
    unsigned int    led_count;
    unsigned char   current_mode;
    unsigned char   current_speed;
    std::string     location;
    std::string     name;
};
