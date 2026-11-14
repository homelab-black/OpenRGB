/*---------------------------------------------------------*\
| ColorfulMBoardController.h                               |
|                                                           |
|   Driver for Colorful USB motherboard lighting            |
|                                                           |
|   OpenRGB contributors                       08 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <hidapi.h>
#include <string>
#include <vector>
#include "RGBController.h"

#define COLORFUL_MBOARD_REPORT_SIZE         65
#define COLORFUL_MBOARD_LEDS_PER_PACKET     20
#define COLORFUL_MBOARD_LED_COUNT           482
#define COLORFUL_MBOARD_DATA_PACKETS        ((COLORFUL_MBOARD_LED_COUNT + COLORFUL_MBOARD_LEDS_PER_PACKET - 1) / COLORFUL_MBOARD_LEDS_PER_PACKET)
#define COLORFUL_MBOARD_COMMAND_DIRECT      0x88
#define COLORFUL_MBOARD_END_SEQUENCE        0xFF

/*---------------------------------------------------------*\
| Native controller modes.  These run in the controller MCU |
| after a single command report.                            |
\*---------------------------------------------------------*/
#define COLORFUL_MBOARD_NATIVE_SLEEP        0x00
#define COLORFUL_MBOARD_NATIVE_BREATHE      0x01
#define COLORFUL_MBOARD_NATIVE_CIRCLE       0x02
#define COLORFUL_MBOARD_NATIVE_RAINBOW      0x03

class ColorfulMBoardController
{
public:
    ColorfulMBoardController(hid_device* dev_handle, const char* path, const std::string& dev_name);
    ~ColorfulMBoardController();

    std::string GetDeviceName();
    std::string GetDeviceLocation();

    void SetLEDs(const std::vector<RGBColor>& colors);

    void SetSleep();
    void SetBreathe(RGBColor color, unsigned char speed);
    void SetCircle(unsigned char brightness, unsigned char speed);
    void SetRainbow(unsigned char brightness, unsigned char speed);

private:
    hid_device* dev;
    std::string location;
    std::string name;

    bool SendPacket(unsigned char command, unsigned char sequence, const RGBColor* colors, unsigned int count);
    bool SendNativePacket(const unsigned char* prefix, unsigned int length);
};
