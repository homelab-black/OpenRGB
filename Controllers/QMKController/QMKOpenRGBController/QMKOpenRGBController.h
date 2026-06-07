/*---------------------------------------------------------*\
| QMKOpenRGBController.h                                    |
|                                                           |
|   Driver for OpenRGB QMK Keyboard Protocol                |
|                                                           |
|   Kasper                                      10 Oct 2020 |
|   Jath03                                      28 May 2021 |
|   reklrekl                                    06 Jun 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <cstring>
#include <map>
#include <hidapi.h>
#include "ResourceManager.h"
#include "RGBController.h"
#include "hsv.h"

#define QMK_OPENRGB_PACKET_SIZE         65
#define QMK_OPENRGB_HID_READ_TIMEOUT    50

enum CommandsId
{
    QMK_OPENRGB_GET_PROTOCOL_VERSION = 1,
    QMK_OPENRGB_GET_QMK_VERSION,
    QMK_OPENRGB_GET_DEVICE_INFO,
    QMK_OPENRGB_GET_MODE_INFO,
    QMK_OPENRGB_GET_LED_INFO,
    QMK_OPENRGB_GET_IS_MODE_ENABLED,
    QMK_OPENRGB_GET_ENABLED_MODES = QMK_OPENRGB_GET_IS_MODE_ENABLED,

    QMK_OPENRGB_SET_MODE,
    QMK_OPENRGB_DIRECT_MODE_SET_SINGLE_LED,
    QMK_OPENRGB_DIRECT_MODE_SET_LEDS,

    QMK_OPENRGB_GET_EFFECT_NAME,
    QMK_OPENRGB_GET_LED_COLOR,


};

enum Modes
{
    QMK_OPENRGB_MODE_OPENRGB_DIRECT = 1,
    QMK_OPENRGB_MODE_SOLID_COLOR,
    QMK_OPENRGB_MODE_ALPHA_MOD,
    QMK_OPENRGB_MODE_GRADIENT_UP_DOWN,
    QMK_OPENRGB_MODE_GRADIENT_LEFT_RIGHT,
    QMK_OPENRGB_MODE_BREATHING,
    QMK_OPENRGB_MODE_BAND_SAT,
    QMK_OPENRGB_MODE_BAND_VAL,
    QMK_OPENRGB_MODE_BAND_PINWHEEL_SAT,
    QMK_OPENRGB_MODE_BAND_PINWHEEL_VAL,
    QMK_OPENRGB_MODE_BAND_SPIRAL_SAT,
    QMK_OPENRGB_MODE_BAND_SPIRAL_VAL,
    QMK_OPENRGB_MODE_CYCLE_ALL,
    QMK_OPENRGB_MODE_CYCLE_LEFT_RIGHT,
    QMK_OPENRGB_MODE_CYCLE_UP_DOWN,
    QMK_OPENRGB_MODE_CYCLE_OUT_IN,
    QMK_OPENRGB_MODE_CYCLE_OUT_IN_DUAL,
    QMK_OPENRGB_MODE_RAINBOW_MOVING_CHEVRON,
    QMK_OPENRGB_MODE_CYCLE_PINWHEEL,
    QMK_OPENRGB_MODE_CYCLE_SPIRAL,
    QMK_OPENRGB_MODE_DUAL_BEACON,
    QMK_OPENRGB_MODE_RAINBOW_BEACON,
    QMK_OPENRGB_MODE_RAINBOW_PINWHEELS,
    QMK_OPENRGB_MODE_RAINDROPS,
    QMK_OPENRGB_MODE_JELLYBEAN_RAINDROPS,
    QMK_OPENRGB_MODE_HUE_BREATHING,
    QMK_OPENRGB_MODE_HUE_PENDULUM,
    QMK_OPENRGB_MODE_HUE_WAVE,
    QMK_OPENRGB_MODE_TYPING_HEATMAP,
    QMK_OPENRGB_MODE_DIGITAL_RAIN,
    QMK_OPENRGB_MODE_SOLID_REACTIVE_SIMPLE,
    QMK_OPENRGB_MODE_SOLID_REACTIVE,
    QMK_OPENRGB_MODE_SOLID_REACTIVE_WIDE,
    QMK_OPENRGB_MODE_SOLID_REACTIVE_MULTIWIDE,
    QMK_OPENRGB_MODE_SOLID_REACTIVE_CROSS,
    QMK_OPENRGB_MODE_SOLID_REACTIVE_MULTICROSS,
    QMK_OPENRGB_MODE_SOLID_REACTIVE_NEXUS,
    QMK_OPENRGB_MODE_SOLID_REACTIVE_MULTINEXUS,
    QMK_OPENRGB_MODE_SPLASH,
    QMK_OPENRGB_MODE_MULTISPLASH,
    QMK_OPENRGB_MODE_SOLID_SPLASH,
    QMK_OPENRGB_MODE_SOLID_MULTISPLASH,
    QMK_OPENRGB_MODE_PIXEL_RAIN,
    QMK_OPENRGB_MODE_PIXEL_FLOW,
    QMK_OPENRGB_MODE_PIXEL_FRACTAL,
    QMK_OPENRGB_MODE_FLOWER_BLOOMING,
    QMK_OPENRGB_MODE_STARLIGHT_SMOOTH,
    QMK_OPENRGB_MODE_STARLIGHT,
    QMK_OPENRGB_MODE_STARLIGHT_DUAL_SAT,
    QMK_OPENRGB_MODE_STARLIGHT_DUAL_HUE,
    QMK_OPENRGB_MODE_RIVERFLOW,


    QMK_OPENRGB_MODE_CUSTOM = 0xFF
};

enum SpeedCommands
{
    QMK_OPENRGB_SPEED_SLOWEST   = 0x00, /* Slowest speed                    */
    QMK_OPENRGB_SPEED_NORMAL    = 0x7F, /* Normal speed                     */
    QMK_OPENRGB_SPEED_FASTEST   = 0xFF, /* Fastest speed                    */
};

enum
{
    QMK_OPENRGB_FAILURE         = 25,   /* Failure status code              */
    QMK_OPENRGB_SUCCESS         = 50,   /* Success status code              */
    QMK_OPENRGB_END_OF_MESSAGE  = 100,  /* End of Message status code       */
};

enum
{
    QMK_OPENRGB_TOTAL_NUMBER_OF_LEDS_BYTE                   = 1,
    QMK_OPENRGB_TOTAL_NUMBER_OF_LEDS_WITH_EMPTY_SPACE_BYTE  = 2
};

enum
{
    QMK_OPENRGB_MODE_BYTE       = 1,
    QMK_OPENRGB_SPEED_BYTE      = 2,
    QMK_OPENRGB_HUE_BYTE        = 3,
    QMK_OPENRGB_SATURATION_BYTE = 4,
    QMK_OPENRGB_VALUE_BYTE      = 5,
};

enum
{
    QMK_OPENRGB_POINT_X_BYTE    = 1,
    QMK_OPENRGB_POINT_Y_BYTE    = 2,
    QMK_OPENRGB_FLAG_BYTE       = 3,
    QMK_OPENRGB_R_COLOR_BYTE    = 4,
    QMK_OPENRGB_G_COLOR_BYTE    = 5,
    QMK_OPENRGB_B_COLOR_BYTE    = 6,
    QMK_OPENRGB_KEYCODE_BYTE    = 7
};

enum
{
    QMK_OPENRGB_FEATURE_SAVE          = (1 << 0),
    QMK_OPENRGB_FEATURE_EFFECT_NAMES  = (1 << 1),
    QMK_OPENRGB_FEATURE_READ              = (1 << 2),
};


// Community/custom effects (unknown IDs) fall back to GetEffectName() over HID.
static const struct { uint8_t id; const char* name; uint8_t flags; } qmk_mode_table[] =
{
    { QMK_OPENRGB_MODE_OPENRGB_DIRECT,         "Direct",                       MODE_FLAG_HAS_PER_LED_COLOR },
    { QMK_OPENRGB_MODE_SOLID_COLOR,            "Static",                       MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_ALPHA_MOD,              "Alphamods",                    MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_GRADIENT_UP_DOWN,       "Gradient Up Down",             MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_GRADIENT_LEFT_RIGHT,    "Gradient Left Right",          MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_BREATHING,              "Breathing",                    MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_BAND_SAT,               "Band Sat",                     MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_BAND_VAL,               "Band Val",                     MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_BAND_PINWHEEL_SAT,      "Band Pinwheel Sat",            MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_BAND_PINWHEEL_VAL,      "Band Pinwheel Val",            MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_BAND_SPIRAL_SAT,        "Band Spiral Sat",              MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_BAND_SPIRAL_VAL,        "Band Spiral Val",              MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_CYCLE_ALL,              "Cycle All",                    MODE_FLAG_HAS_SPEED },
    { QMK_OPENRGB_MODE_CYCLE_LEFT_RIGHT,       "Cycle Left Right",             MODE_FLAG_HAS_SPEED },
    { QMK_OPENRGB_MODE_CYCLE_UP_DOWN,          "Cycle Up Down",                MODE_FLAG_HAS_SPEED },
    { QMK_OPENRGB_MODE_RAINBOW_MOVING_CHEVRON, "Rainbow Moving Chevron",       MODE_FLAG_HAS_SPEED },
    { QMK_OPENRGB_MODE_CYCLE_OUT_IN,           "Cycle Out In",                 MODE_FLAG_HAS_SPEED },
    { QMK_OPENRGB_MODE_CYCLE_OUT_IN_DUAL,      "Cycle Out In Dual",            MODE_FLAG_HAS_SPEED },
    { QMK_OPENRGB_MODE_CYCLE_PINWHEEL,         "Cycle Pinwheel",               MODE_FLAG_HAS_SPEED },
    { QMK_OPENRGB_MODE_CYCLE_SPIRAL,           "Cycle Spiral",                 MODE_FLAG_HAS_SPEED },
    { QMK_OPENRGB_MODE_DUAL_BEACON,            "Dual Beacon",                  MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_RAINBOW_BEACON,         "Rainbow Beacon",               MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_RAINBOW_PINWHEELS,      "Rainbow Pinwheels",            MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_FLOWER_BLOOMING,        "Flower Blooming",              MODE_FLAG_HAS_SPEED },
    { QMK_OPENRGB_MODE_RAINDROPS,              "Raindrops",                    MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_JELLYBEAN_RAINDROPS,    "Jellybean Raindrops",          MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_HUE_BREATHING,          "Hue Breathing",                MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_HUE_PENDULUM,           "Hue Pendulum",                 MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_HUE_WAVE,               "Hue Wave",                     MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_TYPING_HEATMAP,         "Typing Heatmap",               0 },
    { QMK_OPENRGB_MODE_DIGITAL_RAIN,           "Digital Rain",                 0 },
    { QMK_OPENRGB_MODE_SOLID_REACTIVE_SIMPLE,  "Solid Reactive Simple",        MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_SOLID_REACTIVE,         "Solid Reactive",               MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_SOLID_REACTIVE_WIDE,    "Solid Reactive Wide",          MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_SOLID_REACTIVE_MULTIWIDE,"Solid Reactive Multiwide",    MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_SOLID_REACTIVE_CROSS,   "Solid Reactive Cross",         MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_SOLID_REACTIVE_MULTICROSS,"Solid Reactive Multicross",  MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_SOLID_REACTIVE_NEXUS,   "Solid Reactive Nexus",         MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_SOLID_REACTIVE_MULTINEXUS,"Solid Reactive Multinexus",  MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_SPLASH,                 "Splash",                       MODE_FLAG_HAS_SPEED },
    { QMK_OPENRGB_MODE_MULTISPLASH,            "Multisplash",                  MODE_FLAG_HAS_SPEED },
    { QMK_OPENRGB_MODE_SOLID_SPLASH,           "Solid Splash",                 MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_SOLID_MULTISPLASH,      "Solid Multisplash",            MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_PIXEL_RAIN,             "Pixel Rain",                   MODE_FLAG_HAS_SPEED },
    { QMK_OPENRGB_MODE_PIXEL_FLOW,             "Pixel Flow",                   MODE_FLAG_HAS_SPEED },
    { QMK_OPENRGB_MODE_PIXEL_FRACTAL,          "Pixel Fractal",                MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_STARLIGHT,              "Starlight",                    MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_STARLIGHT_SMOOTH,       "Starlight Smooth",             MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_STARLIGHT_DUAL_HUE,     "Starlight Dual Hue",           MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_STARLIGHT_DUAL_SAT,     "Starlight Dual Sat",           MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { QMK_OPENRGB_MODE_RIVERFLOW,              "Riverflow",                    MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR},
    { QMK_OPENRGB_MODE_CUSTOM,                 "Custom ",                      MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR  },
};

struct QMKOpenRGBModeLookup
{
    const char* name;
    uint8_t     flags;
};

inline QMKOpenRGBModeLookup QMKOpenRGBLookupMode(uint8_t openrgb_id)
{
    for (const auto& entry : qmk_mode_table)
    {
        if (entry.id == openrgb_id)
        {
            return { entry.name, entry.flags };
        }
    }
    return { nullptr, 0};
}

typedef struct
{
    uint8_t x;
    uint8_t y;
} point_t;
