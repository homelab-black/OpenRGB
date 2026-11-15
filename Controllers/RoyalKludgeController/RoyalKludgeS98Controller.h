/*---------------------------------------------------------*\
| RoyalKludgeS98Controller.h                                |
|                                                           |
|   Driver for Royal Kludge S98 keyboard                    |
|                                                           |
|   manumyx                                     03 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <mutex>
#include <string>
#include <vector>
#include <hidapi.h>
#include "RGBController.h"

#define ROYAL_KLUDGE_S98_WIRED_REPORT_ID                    0x09
#define ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE                  520
#define ROYAL_KLUDGE_S98_WIRED_HEADER_SIZE                  8

#define ROYAL_KLUDGE_S98_DONGLE_REPORT_ID                   0x13
#define ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE                 20
#define ROYAL_KLUDGE_S98_DONGLE_CHUNK_SIZE                  14

#define ROYAL_KLUDGE_S98_PROFILE_SIZE                       128
#define ROYAL_KLUDGE_S98_EFFECT_TABLE_SIZE                  420
#define ROYAL_KLUDGE_S98_LED_BUFFER_SIZE                    378
#define ROYAL_KLUDGE_S98_MATRIX_CELL_COUNT                  126

#define ROYAL_KLUDGE_S98_CMD_SET_PROFILE                    0x04
#define ROYAL_KLUDGE_S98_CMD_WIRED_SET_LED_COLORS           0x06
#define ROYAL_KLUDGE_S98_CMD_WIRED_SET_LEDS_DIRECT          0x08
#define ROYAL_KLUDGE_S98_CMD_WIRED_SET_LED_EFFECT           0x0A
#define ROYAL_KLUDGE_S98_CMD_WIRED_GET_PASSWORD             0x82
#define ROYAL_KLUDGE_S98_CMD_WIRED_GET_PROFILE              0x84
#define ROYAL_KLUDGE_S98_CMD_WIRED_GET_LED_EFFECT           0x8A

#define ROYAL_KLUDGE_S98_CMD_DONGLE_SET_LED_COLORS          0x02
#define ROYAL_KLUDGE_S98_CMD_DONGLE_GET_PASSWORD            0x05
#define ROYAL_KLUDGE_S98_CMD_DONGLE_GET_STATUS              0x07
#define ROYAL_KLUDGE_S98_CMD_DONGLE_SET_LED_EFFECT          0x09
#define ROYAL_KLUDGE_S98_CMD_DONGLE_GET_PROFILE             0x44

#define ROYAL_KLUDGE_S98_BRIGHTNESS_MIN                     0x00
#define ROYAL_KLUDGE_S98_BRIGHTNESS_MAX                     0x14
#define ROYAL_KLUDGE_S98_BRIGHTNESS_DEFAULT                 0x14

#define ROYAL_KLUDGE_S98_SPEED_MIN                          0x00
#define ROYAL_KLUDGE_S98_SPEED_MAX                          0x04
#define ROYAL_KLUDGE_S98_SPEED_DEFAULT                      0x04

#define ROYAL_KLUDGE_S98_COLOR_FLAG_SPECIFIC                0x00
#define ROYAL_KLUDGE_S98_COLOR_FLAG_RANDOM                  0x07

enum
{
    ROYAL_KLUDGE_S98_MODE_OFF                               = 0x00,
    ROYAL_KLUDGE_S98_MODE_STATIC                            = 0x01,
    ROYAL_KLUDGE_S98_MODE_BREATHING                         = 0x02,
    ROYAL_KLUDGE_S98_MODE_RAINBOW_WAVE                      = 0x03,
    ROYAL_KLUDGE_S98_MODE_FLASH_AWAY                        = 0x04,
    ROYAL_KLUDGE_S98_MODE_RAINDROPS                         = 0x05,
    ROYAL_KLUDGE_S98_MODE_RAINBOW_WHEEL                     = 0x06,
    ROYAL_KLUDGE_S98_MODE_RIPPLE_SHINING                    = 0x07,
    ROYAL_KLUDGE_S98_MODE_STARS_TWINKLE                     = 0x08,
    ROYAL_KLUDGE_S98_MODE_SHADOW_DISAPPEAR                  = 0x09,
    ROYAL_KLUDGE_S98_MODE_RETRO_SNAKE                       = 0x0A,
    ROYAL_KLUDGE_S98_MODE_NEON_STREAM                       = 0x0B,
    ROYAL_KLUDGE_S98_MODE_REACTIVE                          = 0x0C,
    ROYAL_KLUDGE_S98_MODE_SINE_WAVE                         = 0x0D,
    ROYAL_KLUDGE_S98_MODE_SIDE_SCAN                         = 0x0E,
    ROYAL_KLUDGE_S98_MODE_ROTATING_WINDMILL                 = 0x0F,
    ROYAL_KLUDGE_S98_MODE_COLORFUL_WATERFALL                = 0x10,
    ROYAL_KLUDGE_S98_MODE_BLOSSOMING                        = 0x11,
    ROYAL_KLUDGE_S98_MODE_SELF_DEFINE                       = 0x12,
    ROYAL_KLUDGE_S98_MODE_DIRECT                            = 0x13
};

struct RoyalKludgeS98LedSlot
{
    unsigned char   matrix_index;
    RGBColor        color;
};

class RoyalKludgeS98Controller
{
public:
    RoyalKludgeS98Controller(hid_device* dev_handle, const char* path, std::string dev_name, bool is_wireless_mode);
    ~RoyalKludgeS98Controller();

    std::string     GetDeviceLocation();
    std::string     GetNameString();
    std::string     GetSerialString();
    std::string     GetFirmwareVersion();
    bool            IsWireless();

    unsigned char   GetCurrentMode();
    unsigned char   GetEffectBrightness(unsigned char mode);
    unsigned char   GetEffectSpeed(unsigned char mode);
    unsigned char   GetEffectColorFlag(unsigned char mode);
    RGBColor        GetEffectColor(unsigned char mode);

    void            SetMode
                        (
                        unsigned char   mode,
                        unsigned char   brightness,
                        unsigned char   speed,
                        unsigned char   color_flag,
                        RGBColor        color
                        );

    void            SetLedColors(const std::vector<RoyalKludgeS98LedSlot>& led_slots);
    void            SetLedsDirect(const std::vector<RoyalKludgeS98LedSlot>& led_slots);
    void            RestoreMode();

    static bool     ProbeDongleS98(hid_device* dev_handle);

private:
    hid_device*     dev;
    std::string     location;
    std::string     name;
    std::string     version;
    bool            is_wireless;
    std::mutex      io_mutex;

    unsigned char   profile_data[ROYAL_KLUDGE_S98_PROFILE_SIZE];
    unsigned char   effect_data[ROYAL_KLUDGE_S98_EFFECT_TABLE_SIZE];
    unsigned char   last_led_buffer[ROYAL_KLUDGE_S98_LED_BUFFER_SIZE];
    bool            led_buffer_valid;
    unsigned char   last_direct_buffer[ROYAL_KLUDGE_S98_LED_BUFFER_SIZE];
    bool            direct_buffer_valid;
    unsigned char   saved_mode_selection;
    unsigned char   saved_mode;

    static unsigned char CalculateDongleChecksum(const unsigned char* packet);
    static bool     IsS98Password(unsigned int password);

    bool            IsDongleKeyboardOnline();
    bool            ReadPassword();
    bool            ReadProfile();
    bool            ReadEffectTable();

    bool            WriteProfile();
    bool            WriteEffectTable();
    bool            WriteLedBuffer(const unsigned char* led_buffer);

    bool            SendWiredReport(unsigned char cmd_id, const unsigned char* payload, unsigned int length);
    bool            SendWiredDirectReport(const unsigned char* payload, unsigned int length);
    bool            GetWiredReport(unsigned char cmd_id, unsigned char* payload, unsigned int length);

    bool            SendDongleChunked(unsigned char cmd_id, const unsigned char* payload, unsigned int length);
};
