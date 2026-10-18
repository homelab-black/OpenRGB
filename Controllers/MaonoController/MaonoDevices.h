/*---------------------------------------------------------*\
| MaonoDevices.h                                            |
|                                                           |
|   Per-device lighting description for Maono microphones,  |
|   extracted from the Maono Link 4.0.88 device plugins     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBControllerInterface.h"

#define MAONO_VID                       0x352F

#define MAONO_REG_NONE                  0x0000
#define MAONO_VALUE_NONE                0xFFFF
#define MAONO_NO_REPORT_ID              -1

/*---------------------------------------------------------*\
| Two lighting protocols exist across the plugins:          |
|                                                           |
| PRESET_INDEX   (PD200X, PD100X*, DGM20*: class            |
|                RLightWidget) - one register selects the   |
|                colour *and* the effect: static presets,   |
|                pulse presets, colour cycle, specials.     |
|                                                           |
| REGISTER_BLOCK (PD200W, PD100W, DGM20W, T1 Pro, DM40*:    |
|                Analyzer/DataCenter) - separate power,     |
|                brightness, effect and colour registers,   |
|                plus custom colour slots stored as HSV     |
|                (hue x 100, saturation x 100, value x 100) |
\*---------------------------------------------------------*/
enum
{
    MAONO_LIGHT_PRESET_INDEX        = 0,
    MAONO_LIGHT_REGISTER_BLOCK      = 1,
};

struct MaonoColorPreset
{
    const char*     name;
    RGBColor        color;
    unsigned short  value;
};

struct MaonoSpecialEffect
{
    const char*     name;
    unsigned short  value;
};

struct MaonoDeviceInfo
{
    unsigned short              pid;
    const char*                 name;
    const char*                 plugin;             /* Maono Link DLL the data came from     */
    int                         report_id;          /* MAONO_NO_REPORT_ID: frame starts 0xC4 */
    unsigned char               protocol;
    unsigned char               channels;           /* 2 on wireless receivers (TX1/TX2)     */
    unsigned short              channel_stride;     /* register offset of TX2                */

    unsigned short              reg_power;
    unsigned short              reg_brightness;
    unsigned short              reg_effect;
    unsigned short              reg_color;
    unsigned short              reg_custom_count;
    unsigned short              reg_custom_hsv;     /* slot 1 hue; sat, val follow           */
    unsigned char               custom_slots;
    unsigned char               custom_first_index; /* colour index of custom slot 1         */

    unsigned short              brightness_min;
    unsigned short              brightness_max;

    /*-----------------------------------------------------*\
    | REGISTER_BLOCK: values written to reg_effect          |
    \*-----------------------------------------------------*/
    unsigned short              effect_static;
    unsigned short              effect_breathing;
    unsigned short              effect_cycle;
    unsigned short              effect_rhythm;

    /*-----------------------------------------------------*\
    | PRESET_INDEX: value of reg_color for colour cycling   |
    \*-----------------------------------------------------*/
    unsigned short              cycle_color_value;

    const MaonoColorPreset*     static_colors;
    unsigned char               num_static_colors;
    const MaonoColorPreset*     pulse_colors;
    unsigned char               num_pulse_colors;
    const MaonoSpecialEffect*   specials;
    unsigned char               num_specials;

    bool                        tested;             /* confirmed on real hardware            */
};

const MaonoDeviceInfo*  MaonoFindDevice(unsigned short pid);
unsigned int            MaonoDeviceCount();
const MaonoDeviceInfo*  MaonoDeviceAt(unsigned int index);
