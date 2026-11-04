/*---------------------------------------------------------*\
| AkkoController.h                                          |
|                                                           |
|   Driver for Akko keyboard                                |
|                                                           |
|   Bizzotto Nicola (nicola02nb)                05 Feb 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "KeyboardLayoutManager.h"
#include "RGBController.h"
#include <string>
#include <hidapi.h>

#define AKKO_PACKET_LENGTH                      0x40
#define AKKO_SET_KEYBOARD_EFFECT                0x07
#define AKKO_SET_LIGHTSTRIP_EFFECT              0x08
#define AKKO_COMMAND_PING                       0xF7
#define HID_MAX_STR                             255

enum AKKO_ZONE
{
    Z_KEYBOARD        = 0,
    Z_LIGHT_STRIPS    = 1,
};

enum AKKO_ZONE_SIZE
{
    Z_KEYBOARD_SIZE       = 130,
};

enum AKKO_BYTE_INDEX
{
    COMMAND         = 1,
    MODE            = 2,
    SPEED           = 3,
    BRIGHTNESS      = 4,
    FLAGS           = 5,
    RED             = 6,
    GREEN           = 7,
    BLUE            = 8,
    CUSTOM_FILLER   = 8,
    EFFECT_FILLER   = 9,
};

enum AKKO_BYTE_COMMAND
{
    PING            = 0xF7,
    SET             = 0x00,
    GET             = 0x80,
    KEYBOARD        = 0x07,
    LIGHT_STRIPS    = 0x08,
    CUSTOM_KEYBOARD = 0x0c,
    DIRECT_MUSIC    = 0x0d,
    DIRECT_COLOR    = 0x0e,
};

enum AKKO_MODE
{
    OFF                 = 0x00,
    ALWAYS_ON           = 0x01,//Used as Static
    BREATH              = 0x02,
    METEOR              = 0x12,
    RIPPLE              = 0x05,
    AURORA              = 0x17,
    RAINDROP            = 0x11,
    COVERAGE            = 0x09,
    STEADY_STREAM       = 0x07,
    NEON                = 0x03,
    WAVE                = 0x04,
    STARS_TWINKLE       = 0x06,
    LIKE_SHADOWS        = 0x08,
    SINE_WAVE           = 0x0a,
    FLOWING_SPRING		= 0x0b,
    FLOWERS_BLOOMING    = 0x0c,
    LASER               = 0x0e,
    PEAK_TURN           = 0x0f,
    INCLINED_RAIN       = 0x10,
    LIGHT_TRACE         = 0x13,
    ENDLESS             = 0x18,
    USER_PICTURE        = 0x0d,//Used as Custom
    MUSIC_FOLLOW        = 0x14,
    MUSIC_FOLLOW_2		= 0x16,
    LIGHT_SHADOW        = 0x15,//Used as Direct
};

enum AKKO_SPEED
{
    S_MIN       = 0x00,
    S_MAX       = 0x04,
    S_DEFAULT	= 0x02,
};

enum AKKO_BRIGHTNESS
{
    B_MIN       = 0x00,
    B_MAX       = 0x04,
    B_DEFAULT   = 0x04,
};

enum AKKO_FLAGS
{
    DAZZLE_MUSIC_FOLLOW_OFF     = 0x04,
    DAZZLE_MUSIC_FOLLOW_ON      = 0x00,
    DAZZLE_ON                   = 0x07,
    DAZZLE_OFF                  = 0x08,
    DEFAULT                     = 0x00,
    DRIFT_RIGHT                 = 0X00,
    DRIFT_LEFT                  = 0X10,
    DRIFT_DOWN                  = 0X20,
    DRIFT_UP                    = 0X30,
    STEADY_STREAM_ZIG_ZAG       = 0x00,
    STEADY_STREAM_RETURN        = 0x10,
    CAISPRING_SURGING_OUT       = 0x00,
    CAISPRING_SURGING_IN        = 0x10,
    FLOWERS_BLOOMING_RIGHT		= 0x00,
    FLOWERS_BLOOMING_LEFT       = 0x10,
    PEAK_TURN_ANTI_CLOCKWISE    = 0x00,
    PEAK_TURN_CLOCKWISE         = 0x10,
    MUSIC_UPRIGHT               = 0x00,
    MUSIC_SEPARATE              = 0x10,
    MUSIC_INTERSECT             = 0x20,
};

struct AkkoZoneInfo
{
    std::string             name;
    AKKO_ZONE               zone;
    AKKO_MODE               mode;
    AKKO_SPEED              speed;
    AKKO_BRIGHTNESS         brightness;
    AKKO_FLAGS              flags;
    RGBColor                color;
    int                     leds_count;
    std::vector<RGBColor>   colors;
};

class AkkoController
{
public:
    AkkoController(hid_device* dev_handle, char *_path, bool is_wireless, bool has_light_strips);
    ~AkkoController();

    std::string GetDeviceName();
    std::string GetSerial();
    std::string GetLocation();

    KEYBOARD_LAYOUT GetKeyboardLayout();
    KEYBOARD_SIZE GetKeyboardSize();
    keyboard_keymap_overlay_values GetKeyboardLayoutValues();

    AkkoZoneInfo GetZoneInfo(AKKO_ZONE zone);

    void SetZoneEffect(const AkkoZoneInfo& zone_info);
    void SetCustomLeds(const AkkoZoneInfo& zone_info);
    void SetDirectMusic(const AkkoZoneInfo& zone_info);
    void SetDirectColor(const AkkoZoneInfo& zone_info);

    bool HasLightStrips();

private:
    std::string device_name;
    std::string location;
    hid_device* dev;

    bool                                is_wireless;
    bool                                has_light_strips;

    std::vector<RGBColor> getLedsData(AKKO_ZONE zone);
    static AKKO_BYTE_COMMAND getByteFromZone(AKKO_ZONE zone);
    static std::string getZoneName(AKKO_ZONE zone);
};

bool IsZoneCompatibleWith(AKKO_ZONE zone, AKKO_MODE mode);
