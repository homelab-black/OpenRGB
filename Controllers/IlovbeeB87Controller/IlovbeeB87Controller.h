/*---------------------------------------------------------*\
| IlovbeeB87Controller.h                                   |
|                                                           |
|   Driver for ilovbee B87 keyboard lighting                |
|                                                           |
|   w568w                                       18 Jul 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <hidapi.h>
#include <string>
#include "RGBController.h"

#define ILOVBEE_B87_REPORT_ID                         0x06
#define ILOVBEE_B87_REPORT_SIZE                       520
#define ILOVBEE_B87_HEADER_SIZE                       8
#define ILOVBEE_B87_LED_CONFIG_SIZE                   128
#define ILOVBEE_B87_RGB_TABLE_SIZE                    420
#define ILOVBEE_B87_RGB_COLORS_PER_MODE               7

#define ILOVBEE_B87_READ_LED_CONFIG_COMMAND           0x84
#define ILOVBEE_B87_WRITE_LED_CONFIG_COMMAND          0x04
#define ILOVBEE_B87_READ_RGB_TABLE_COMMAND            0x8A
#define ILOVBEE_B87_WRITE_RGB_TABLE_COMMAND           0x0A

#define ILOVBEE_B87_CUSTOM_MODE_INDEX                 0x09
#define ILOVBEE_B87_CURRENT_MODE_INDEX                0x0A
#define ILOVBEE_B87_MODE_CONFIG_BASE                  0x38
#define ILOVBEE_B87_CONFIG_SIGNATURE_INDEX            0x7E

#define ILOVBEE_B87_BRIGHTNESS_MIN                    0
#define ILOVBEE_B87_BRIGHTNESS_MAX                    4
#define ILOVBEE_B87_SPEED_MIN                         1
#define ILOVBEE_B87_SPEED_MAX                         4

enum
{
    ILOVBEE_B87_MODE_OFF               = 0,
    ILOVBEE_B87_MODE_STATIC            = 1,
    ILOVBEE_B87_MODE_BREATHING         = 2,
    ILOVBEE_B87_MODE_RAINBOW           = 3,
    ILOVBEE_B87_MODE_FLASH_AWAY        = 4,
    ILOVBEE_B87_MODE_RAINDROPS         = 5,
    ILOVBEE_B87_MODE_RAINBOW_WHEEL     = 6,
    ILOVBEE_B87_MODE_RIPPLES_SHINING   = 7,
    ILOVBEE_B87_MODE_STARS_TWINKLE     = 8,
    ILOVBEE_B87_MODE_SHADOW_DISAPPEAR  = 9,
    ILOVBEE_B87_MODE_RETRO_SNAKE       = 10,
    ILOVBEE_B87_MODE_NEON_STREAM       = 11,
    ILOVBEE_B87_MODE_REACTION          = 12,
    ILOVBEE_B87_MODE_SINE_WAVE         = 13,
    ILOVBEE_B87_MODE_HIDDEN_SCAN       = 14,
    ILOVBEE_B87_MODE_ROTATING_WINDMILL = 15,
    ILOVBEE_B87_MODE_WATERFALL         = 16,
    ILOVBEE_B87_MODE_BLOSSOMING        = 17,
};

class IlovbeeB87Controller
{
public:
    IlovbeeB87Controller(hid_device* dev_handle, const hid_device_info& info, std::string dev_name);
    ~IlovbeeB87Controller();

    bool            Initialize();

    std::string     GetDeviceLocation();
    std::string     GetDeviceName();
    std::string     GetSerialString();

    unsigned char   GetCurrentMode();
    unsigned char   GetModeBrightness(unsigned char mode_value);
    unsigned char   GetModeSpeed(unsigned char mode_value);
    RGBColor        GetModeColor(unsigned char mode_value);

    bool            SetMode(unsigned char mode_value, unsigned int brightness, bool update_speed, unsigned int speed, bool update_color, RGBColor color);

private:
    hid_device*     dev;
    std::string     location;
    std::string     name;

    unsigned char   led_config[ILOVBEE_B87_LED_CONFIG_SIZE] = { 0 };
    unsigned char   rgb_table[ILOVBEE_B87_RGB_TABLE_SIZE]    = { 0 };

    bool            ReadG5Data(unsigned char command, unsigned char* payload, unsigned int payload_size);
    bool            SendG5Command(unsigned char command, const unsigned char* payload, unsigned int payload_size);
    bool            ReadLightingState();
    bool            VerifyLightingState(const unsigned char* expected_led_config, const unsigned char* expected_rgb_table);
};
