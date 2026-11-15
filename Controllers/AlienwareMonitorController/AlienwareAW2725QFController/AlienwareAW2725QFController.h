/*---------------------------------------------------------*\
| AlienwareAW2725QFController.h                             |
|                                                           |
|   Driver for the Alienware AW2725QF monitor               |
|                                                           |
|   Ferréol DUBOIS COLI (Fefe_du_973)           23 Jan 2025 |
|   Jingcheng Tang (at1289)                     08 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <hidapi.h>
#include <vector>
#include <string>
#include "RGBController.h"

#define AW2725QF_MODE_DIRECT        0x00
#define AW2725QF_MODE_MORPH         0x01
#define AW2725QF_MODE_PULSE         0x02
#define AW2725QF_MODE_SPECTRUM      0x03
#define AW2725QF_MODE_STATIC        0x04
#define AW2725QF_MODE_BREATHING     0x05

#define AW2725QF_ZONE_LOGO          0x02
#define AW2725QF_ZONE_NUMBER        0x01
#define AW2725QF_ZONE_POWER         0x04
#define AW2725QF_ZONE_ALL           0x07

#define AW2725QF_BRIGHTNESS_MAX     0xFF
#define AW2725QF_SPEED_SLOW         0
#define AW2725QF_SPEED_MEDIUM       1
#define AW2725QF_SPEED_FAST         2

class AlienwareAW2725QFController
{
public:
    AlienwareAW2725QFController(hid_device* dev_handle, const char* path);
    ~AlienwareAW2725QFController();

    std::string GetLocation();
    std::string GetSerialString();
    bool SetMode(unsigned char mode, unsigned char led_mask, RGBColor color1, RGBColor color2, unsigned char brightness, unsigned char speed);

private:
    hid_device* dev;
    std::string location;

    bool PerformLogin();
    std::vector<unsigned char> GenerateKey(const std::vector<unsigned char>& response);
    bool SendControlPacket(const unsigned char* data, size_t length);
    bool SendDDCCommand(const unsigned char* data, size_t length);
    std::vector<unsigned char> GetReportResponse();
};
