/*-------------------------------------------------------------------*\
| CMMM731Controller.h                                                 |
|                                                                     |
|   Driver for the Cooler Master MM731 Mouse                          |
|                                                                     |
|   Chris M (Dr_No)                              1 Jan 2022           |
|                                                                     |
|   This file is part of the OpenRGB project                          |
|   SPDX-License-Identifier: GPL-2.0-only                             |
\*-------------------------------------------------------------------*/

#include <string>
#include <hidapi.h>
#include "RGBController.h"

#pragma once

#define CM_MM731_INTERRUPT_TIMEOUT  250
#define CM_MM731_PACKET_SIZE        65
#define CM_MM731_BRIGHTNESS_MIN     0
#define CM_MM731_BRIGHTNESS_MAX     255
#define HID_MAX_STR 255

enum
{
    CM_MM731_MODE_STATIC            = 0x00, //Static Mode - Set entire zone to a single color.
    CM_MM731_MODE_BREATHING         = 0x01, //Breathing Mode - Fades between fully off and fully on.
    CM_MM731_MODE_SPECTRUM          = 0x02, //Spectrum Cycle Mode - Cycles through the color spectrum on all lights on the device
    CM_MM731_MODE_OFF               = 0x03, //Turn off - All leds off
};

enum
{
    CM_MM731_REPORT_BYTE            = 1,
    CM_MM731_COMMAND_BYTE           = 2,
    CM_MM731_FUNCTION_BYTE          = 3,
    CM_MM731_MODE_BYTE              = 4,
    CM_MM731_NFI_1                  = 5,
    CM_MM731_BRIGHTNESS_BYTE        = 6,
    CM_MM731_SPEED_BYTE             = 7,
    CM_MM731_RED_BYTE               = 8,
    CM_MM731_GREEN_BYTE             = 9,
    CM_MM731_BLUE_BYTE              = 10,
    CM_MM731_NFI_2                  = 11,
};

enum
{
    CM_MM731_SPEED_SLOWEST          = 0x00, // Slowest speed
    CM_MM731_SPEED_SLOW             = 0x01, // Slower speed
    CM_MM731_SPEED_NORMAL           = 0x02, // Normal speed
    CM_MM731_SPEED_FAST             = 0x03, // Fast speed
    CM_MM731_SPEED_FASTEST          = 0x04, // Fastest speed
};

class CMMM731Controller
{
public:
    CMMM731Controller(hid_device* dev_handle, char *_path);
    ~CMMM731Controller();

    std::string     GetDeviceName();
    std::string     GetSerial();
    std::string     GetLocation();

    uint8_t         GetMode();
    uint8_t         GetLedSpeed();

    uint8_t         GetLedRed();
    uint8_t         GetLedGreen();
    uint8_t         GetLedBlue();
    uint8_t         GetBrightness();
    bool            GetRandomColours();

    void            SetMode(uint8_t mode, uint8_t speed, uint8_t brightness, RGBColor colour, bool random_colours);
    void            SetLedsDirect(RGBColor logo_colour);
    void            SendSavePacket();
private:
    std::string     device_name;
    std::string     serial;
    std::string     location;
    hid_device*     dev;

    uint8_t         zone_index;
    uint8_t         current_mode;
    uint8_t         current_speed;

    uint8_t         current_red;
    uint8_t         current_green;
    uint8_t         current_blue;
    uint8_t         current_brightness;
    bool            bool_random;

    void            GetStatus();
    void            SendUpdate();

    //void        GetColourStatus();
    //void        GetCustomStatus();
    void            SendInitPacket();
    void            SendApplyPacket();
};
