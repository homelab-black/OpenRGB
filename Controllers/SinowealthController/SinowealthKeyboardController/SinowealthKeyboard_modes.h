/*---------------------------------------------------------*\
| SinowealthKeyboard_modes.h                                |
|                                                           |
|   Shared values from Sinowealth Keyboard Controllers      |
|   Moved out to avoid duplication and header conflicts     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

enum
{
    MODE_OFF                    = 0x0,
    MODE_STATIC                 = 0x1,
    MODE_RESPIRE                = 0x2,
    MODE_RAINBOW                = 0x3,
    MODE_FLASH_AWAY             = 0x4,
    MODE_RAINDROPS              = 0x5,
    MODE_RAINBOW_WHEEL          = 0x6,
    MODE_RIPPLES_SHINING        = 0x7,
    MODE_STARS_TWINKLE          = 0x8,
    MODE_SHADOW_DISAPPEAR       = 0x9,
    MODE_RETRO_SNAKE            = 0xA,
    MODE_NEON_STREAM            = 0xB,
    MODE_REACTION               = 0xC,
    MODE_SINE_WAVE              = 0xD,
    MODE_RETINUE_SCANNING       = 0xE,
    MODE_ROTATING_WINDMILL      = 0xF,
    MODE_COLORFUL_WATERFALL     = 0x10,
    MODE_BLOSSOMING             = 0x11,
    MODE_ROTATING_STORM         = 0x12,
    MODE_COLLISION              = 0x13,
    MODE_PERFECT                = 0x14,
    MODE_PER_KEY                = 0x15
};

enum
{
    SPEED_SLOW                  = 0x12,
    SPEED_NORMAL                = 0x22,
    SPEED_FASTER                = 0x32,
    SPEED_FASTEST               = 0x42,
};

enum
{
    BRIGHTNESS_OFF              = 0x0,
    BRIGHTNESS_QUARTER          = 0x1,
    BRIGHTNESS_HALF             = 0x2,
    BRIGHTNESS_THREE_QUARTERS   = 0x3,
    BRIGHTNESS_FULL             = 0x4
};
