/*---------------------------------------------------------*\
| LogitechG300Controller.h                                  |
|                                                           |
|   Driver for Logitech G300s Optical Gaming Mouse          |
|                                                           |
|   Aman Ali                                  08 Sep 2026   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi.h>
#include "RGBController.h"

/*---------------------------------------------------------------------------------------------*\
| The illumination is driven by three single bit channels, so the eight reachable colors are the |
| corners of the RGB cube.  Any other color is snapped to the nearest corner.                     |
\*---------------------------------------------------------------------------------------------*/
#define LOGITECH_G300_LED_RED                       0x01
#define LOGITECH_G300_LED_GREEN                     0x02
#define LOGITECH_G300_LED_BLUE                      0x04

/*---------------------------------------------------------------------------------------------*\
| Report 0xF1 is the live LED register and is volatile, so it is safe to write at effect update  |
| rates.  Reports 0xF3 - 0xF5 are the three onboard profile records and persist across a power   |
| cycle.  Report 0xF0 reports which profile is currently active.                                 |
\*---------------------------------------------------------------------------------------------*/
#define LOGITECH_G300_REPORT_ID_ACTIVE              0xF0
#define LOGITECH_G300_REPORT_ID_LED                 0xF1
#define LOGITECH_G300_REPORT_ID_PROFILE_0           0xF3

#define LOGITECH_G300_REPORT_SIZE                   35
#define LOGITECH_G300_PROFILE_MAX                   2

/*---------------------------------------------------------------------------------------------*\
| Upper five bits of the profile record's color byte are not understood, so they are preserved   |
| when the LED bits are patched.                                                                 |
\*---------------------------------------------------------------------------------------------*/
#define LOGITECH_G300_LED_PRESERVE_MASK             0xF8

enum
{
    LOGITECH_G300_MODE_DIRECT                       = 0x00
};

class LogitechG300Controller
{
public:
    LogitechG300Controller(hid_device* dev, const char* path, std::string dev_name);
    ~LogitechG300Controller();

    std::string     GetNameString();
    std::string     GetSerialString();

    void            SetLED(RGBColor color);
    void            Save(RGBColor color);

private:
    unsigned char   ColorToBits(RGBColor color);

    hid_device*     dev;
    std::string     location;
    std::string     name;
};
