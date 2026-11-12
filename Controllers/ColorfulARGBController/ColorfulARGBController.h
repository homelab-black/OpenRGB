/*---------------------------------------------------------*\
| ColorfulARGBController.h                                  |
|                                                             |
|   Driver for the onboard ARGB header on the Colorful       |
|   B650M Battle-AX Plus V14. Protocol reverse-engineered    |
|   from the official iGame Center software (see the .cpp    |
|   comments for commit/sync cycle details).                 |
|                                                             |
|   This file is part of the OpenRGB project                 |
|   SPDX-License-Identifier: GPL-2.0-only                    |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>

#define COLORFUL_ARGB_NUM_ZONES     20
#define COLORFUL_ARGB_REPORT_LEN    64

class ColorfulARGBController
{
public:
    ColorfulARGBController(hid_device* dev_handle, const char* path);
    ~ColorfulARGBController();

    std::string GetDeviceLocation();
    std::string GetSerialString();

    /*-----------------------------------------------------*\
    | colors_rgb: flat array [R,G,B, R,G,B, ...] of length   |
    | num_zones*3 bytes. Sends the full cycle (10 data       |
    | packets + 1 commit packet) then returns.               |
    \*-----------------------------------------------------*/
    void SetColors(unsigned char* colors_rgb, unsigned int num_zones);

private:
    hid_device*     dev;
    std::string     location;

    void SendPacket(unsigned char* zone_rgb, unsigned char seq);
};
