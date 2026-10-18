/*---------------------------------------------------------*\
| MaonoDevices.cpp                                          |
|                                                           |
|   Lighting tables for Maono microphones. Every value      |
|   here was read out of the Maono Link 4.0.88 plugin DLL   |
|   named in the entry; addresses of the functions used as  |
|   evidence are listed in docs/PROTOCOL.md                 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "MaonoDevices.h"

#define MAONO_ARRAY_COUNT(a)  ((unsigned char)(sizeof(a) / sizeof((a)[0])))

/*---------------------------------------------------------*\
| PRESET_INDEX colour lists (order of the colour buttons /  |
| QString list in each RLightWidget constructor)            |
\*---------------------------------------------------------*/

/* PD200X.dll 0x180026C50, PD100X_LITE.dll, DGM20*.dll 0x1800178C0 */
static const MaonoColorPreset legacy8_static[] =
{
    { "White",      ToRGBColor(0xFF, 0xFF, 0xFF),   0 },
    { "Red",        ToRGBColor(0xFF, 0x00, 0x00),   1 },
    { "Orange",     ToRGBColor(0xFF, 0x80, 0x00),   2 },
    { "Yellow",     ToRGBColor(0xFF, 0xFF, 0x00),   3 },
    { "Green",      ToRGBColor(0x00, 0xFF, 0x00),   4 },
    { "Blue",       ToRGBColor(0x00, 0x00, 0xFF),   5 },
    { "Light Blue", ToRGBColor(0x00, 0xBF, 0xFF),   6 },
    { "Purple",     ToRGBColor(0x80, 0x00, 0xFF),   7 },
};                                                  /* 8 = blend (cycle) */

/* PD100X.dll 0x180016790: blend, red ... purple, white; pulse = +0x10 (0x180019490) */
static const MaonoColorPreset pd100x_static[] =
{
    { "Red",        ToRGBColor(0xFF, 0x00, 0x00),   1 },
    { "Orange",     ToRGBColor(0xFF, 0x80, 0x00),   2 },
    { "Yellow",     ToRGBColor(0xFF, 0xFF, 0x00),   3 },
    { "Green",      ToRGBColor(0x00, 0xFF, 0x00),   4 },
    { "Blue",       ToRGBColor(0x00, 0x00, 0xFF),   5 },
    { "Light Blue", ToRGBColor(0x00, 0xBF, 0xFF),   6 },
    { "Purple",     ToRGBColor(0x80, 0x00, 0xFF),   7 },
    { "White",      ToRGBColor(0xFF, 0xFF, 0xFF),   8 },
};                                                  /* 0 = blend (cycle) */

static const MaonoColorPreset pd100x_pulse[] =
{
    { "Red",        ToRGBColor(0xFF, 0x00, 0x00),   0x11 },
    { "Orange",     ToRGBColor(0xFF, 0x80, 0x00),   0x12 },
    { "Yellow",     ToRGBColor(0xFF, 0xFF, 0x00),   0x13 },
    { "Green",      ToRGBColor(0x00, 0xFF, 0x00),   0x14 },
    { "Blue",       ToRGBColor(0x00, 0x00, 0xFF),   0x15 },
    { "Light Blue", ToRGBColor(0x00, 0xBF, 0xFF),   0x16 },
    { "Purple",     ToRGBColor(0x80, 0x00, 0xFF),   0x17 },
    { "White",      ToRGBColor(0xFF, 0xFF, 0xFF),   0x18 },
};

/* PD100X_RGB.dll 0x180016780: orange, green, blue, purple, blend; pulse = +5 (0x180019C30) */
static const MaonoColorPreset pd100x_rgb_static[] =
{
    { "Orange",     ToRGBColor(0xFF, 0x80, 0x00),   0 },
    { "Green",      ToRGBColor(0x00, 0xFF, 0x00),   1 },
    { "Blue",       ToRGBColor(0x00, 0x00, 0xFF),   2 },
    { "Purple",     ToRGBColor(0x80, 0x00, 0xFF),   3 },
};                                                  /* 4 = blend (cycle) */

static const MaonoColorPreset pd100x_rgb_pulse[] =
{
    { "Orange",     ToRGBColor(0xFF, 0x80, 0x00),   5 },
    { "Green",      ToRGBColor(0x00, 0xFF, 0x00),   6 },
    { "Blue",       ToRGBColor(0x00, 0x00, 0xFF),   7 },
    { "Purple",     ToRGBColor(0x80, 0x00, 0xFF),   8 },
};

/* PD100X_RGB.dll 0x180019620: "Dynamic" tab (动态Ⅰ, 动态Ⅱ, 水平仪) */
static const MaonoSpecialEffect pd100x_rgb_specials[] =
{
    { "Dynamic I",      10 },
    { "Dynamic II",     11 },
    { "Level Meter",    12 },
};

/*---------------------------------------------------------*\
| REGISTER_BLOCK preset lists                               |
\*---------------------------------------------------------*/

/* QColor table built in the light page (PD200XW.dll 0x180024E10, same in */
/* PD100XW, DGM20W, T1Pro, DM40L_GEN2)                                     */
static const MaonoColorPreset block_static[] =
{
    { "White",      ToRGBColor(0xFF, 0xFF, 0xFF),   0 },
    { "Red",        ToRGBColor(0xFF, 0x00, 0x00),   1 },
    { "Orange",     ToRGBColor(0xFF, 0xA5, 0x00),   2 },
    { "Yellow",     ToRGBColor(0xFF, 0xFF, 0x00),   3 },
    { "Green",      ToRGBColor(0x00, 0xFF, 0x00),   4 },
    { "Cyan",       ToRGBColor(0x00, 0xFF, 0xFF),   5 },
    { "Blue",       ToRGBColor(0x00, 0x00, 0xFF),   6 },
    { "Magenta",    ToRGBColor(0xFF, 0x00, 0xFF),   7 },
};

/* DM40.dll / DM40L.dll: btn_light_* added to the colour group 0x180029AB0 */
static const MaonoColorPreset dm40_static[] =
{
    { "White",      ToRGBColor(0xFF, 0xFF, 0xFF),   0 },
    { "Cyan",       ToRGBColor(0x00, 0xFF, 0xFF),   1 },
    { "Blue",       ToRGBColor(0x00, 0x00, 0xFF),   2 },
    { "Red",        ToRGBColor(0xFF, 0x00, 0x00),   3 },
    { "Orange",     ToRGBColor(0xFF, 0xA5, 0x00),   4 },
    { "Yellow",     ToRGBColor(0xFF, 0xFF, 0x00),   5 },
    { "Purple",     ToRGBColor(0x80, 0x00, 0xFF),   6 },
    { "Green",      ToRGBColor(0x00, 0xFF, 0x00),   7 },
};

/*---------------------------------------------------------*\
| Entry helpers                                             |
\*---------------------------------------------------------*/
#define MAONO_N                     MAONO_VALUE_NONE

#define LEGACY(pid, name, dll, rid, bright_reg, bright_max, statics, cycle, pulse, num_pulse, spec, num_spec) \
    { pid, name, dll, rid, MAONO_LIGHT_PRESET_INDEX, 1, 0,                                     \
      0x2036, bright_reg, MAONO_REG_NONE, 0x2038, MAONO_REG_NONE, MAONO_REG_NONE, 0, 0,          \
      0, bright_max, MAONO_N, MAONO_N, MAONO_N, MAONO_N, cycle,                                                          \
      statics, MAONO_ARRAY_COUNT(statics), pulse, num_pulse, spec, num_spec, false }

/* Analyzer register map 0x2089.. (PD200XW.dll 0x180069D30): power, brightness, effect, */
/* colour, -, custom count, custom HSV x2; effects 0 static / 1 cycle / 2 breathing      */
#define LIGHTBLOCK(pid, name, dll, rid, channels, tested) \
    { pid, name, dll, rid, MAONO_LIGHT_REGISTER_BLOCK, channels, 0x0800,                         \
      0x2089, 0x208A, 0x208B, 0x208C, 0x208E, 0x208F, 2, 8,                                      \
      1, 100, 0, 2, 1, MAONO_N, MAONO_N,                                                                     \
      block_static, MAONO_ARRAY_COUNT(block_static), nullptr, 0, nullptr, 0, tested }

/* DM40::DataCenter register map (DM40.dll 0x18000BE00): same block, 10 custom slots   */
#define DM40(pid, name, dll) \
    { pid, name, dll, 0x00, MAONO_LIGHT_REGISTER_BLOCK, 1, 0,                                    \
      0x2089, 0x208A, 0x208B, 0x208C, 0x208E, 0x208F, 10, 8,                                     \
      0, 100, 0, 2, 1, MAONO_N, MAONO_N,                                                                     \
      dm40_static, MAONO_ARRAY_COUNT(dm40_static), nullptr, 0, nullptr, 0, false }

/* DM40L_GEN2.dll Analyzer map 0x18008EF70: 0x206E.. ; effects 0 fixed / 1 breathing /  */
/* 2 cycle / 3 rhythm (0x180043F50 sends the button id as-is); no custom colours        */
#define DM40G2(pid, name) \
    { pid, name, "DM40L_GEN2.dll", 0x00, MAONO_LIGHT_REGISTER_BLOCK, 1, 0,                       \
      0x206E, 0x206F, 0x2070, 0x2071, MAONO_REG_NONE, MAONO_REG_NONE, 0, 0,                       \
      1, 100, 0, 1, 2, 3, MAONO_N,                                                                     \
      block_static, MAONO_ARRAY_COUNT(block_static), nullptr, 0, nullptr, 0, false }

static const MaonoDeviceInfo maono_devices[] =
{
    /*-----------------------------------------------------*\
    | PRESET_INDEX family                                   |
    \*-----------------------------------------------------*/
    LEGACY(0x0104, "Maono PD200X",          "PD200X.dll",       0x4B, 0x2037, 100, legacy8_static,    8, nullptr,          0,                         nullptr,             0),
    LEGACY(0x010C, "Maono PD100X",          "PD100X.dll",       0x4B, 0x2037, 100, pd100x_static,     0, pd100x_pulse,     MAONO_ARRAY_COUNT(pd100x_pulse), nullptr,             0),
    LEGACY(0x0108, "Maono PD100X RGB",      "PD100X_RGB.dll",   0x4B, 0x2037, 100, pd100x_rgb_static, 4, pd100x_rgb_pulse, MAONO_ARRAY_COUNT(pd100x_rgb_pulse), pd100x_rgb_specials, MAONO_ARRAY_COUNT(pd100x_rgb_specials)),
    LEGACY(0x0109, "Maono PD100X Lite",     "PD100X_LITE.dll",  0x4B, 0x2037, 100, legacy8_static,    8, nullptr,          0,                         nullptr,             0),
    LEGACY(0x0105, "Maono DGM20",           "DGM20_OLD.dll",    0x51, 0x202A,  99, legacy8_static,    8, nullptr,          0,                         nullptr,             0),
    LEGACY(0x0112, "Maono DGM20",           "DGM20.dll",        0x51, 0x202A,  99, legacy8_static,    8, nullptr,          0,                         nullptr,             0),
    LEGACY(0x0113, "Maono DGM20G",          "DGM20G.dll",       0x51, 0x202A,  99, legacy8_static,    8, nullptr,          0,                         nullptr,             0),

    /*-----------------------------------------------------*\
    | REGISTER_BLOCK family - wireless (Analyzer)           |
    \*-----------------------------------------------------*/
    LIGHTBLOCK(0x0411, "Maono PD200W",                  "PD200XW.dll",  0x00, 1, true),
    LIGHTBLOCK(0x040E, "Maono PD200W Receiver",         "PD200XW.dll",  0x00, 2, false),
    LIGHTBLOCK(0x040F, "Maono PD200W Receiver",         "PD200XW.dll",  0x00, 2, false),
    LIGHTBLOCK(0x0410, "Maono PD200W Receiver",         "PD200XW.dll",  0x00, 2, false),
    LIGHTBLOCK(0x0417, "Maono PD100W",                  "PD100XW.dll",  0x00, 1, false),
    LIGHTBLOCK(0x0414, "Maono PD100W Receiver",         "PD100XW.dll",  0x00, 2, false),
    LIGHTBLOCK(0x0415, "Maono PD100W Receiver",         "PD100XW.dll",  0x00, 2, false),
    LIGHTBLOCK(0x0416, "Maono PD100W Receiver",         "PD100XW.dll",  0x00, 2, false),
    LIGHTBLOCK(0x041D, "Maono DGM20W",                  "DGM20W.dll",   0x00, 1, false),
    LIGHTBLOCK(0x041A, "Maono DGM20W Receiver",         "DGM20W.dll",   0x00, 2, false),
    LIGHTBLOCK(0x041B, "Maono DGM20W Receiver",         "DGM20W.dll",   0x00, 2, false),
    LIGHTBLOCK(0x041C, "Maono DGM20W Receiver",         "DGM20W.dll",   0x00, 2, false),
    LIGHTBLOCK(0x0418, "Maono Wave T1 Mini Pro",        "T1Pro.dll",    MAONO_NO_REPORT_ID, 1, false),
    LIGHTBLOCK(0x0419, "Maono Wave T1 Mini Pro",        "T1Pro.dll",    MAONO_NO_REPORT_ID, 1, false),

    /*-----------------------------------------------------*\
    | REGISTER_BLOCK family - DM40 (DataCenter)             |
    \*-----------------------------------------------------*/
    DM40(0x0405, "Maono DM40 Pro",      "DM40.dll"),
    DM40(0x0406, "Maono DM40 Pro",      "DM40.dll"),
    DM40(0x040C, "Maono DM40",          "DM40.dll"),
    DM40(0x040D, "Maono DM40",          "DM40.dll"),
    DM40(0x0412, "Maono DM40 Live",     "DM40L.dll"),
    DM40(0x0413, "Maono DM40 Live",     "DM40L.dll"),
    DM40(0x0420, "Maono DM40 Live",     "DM40L.dll"),
    DM40(0x0421, "Maono DM40 Live",     "DM40L.dll"),
    DM40(0x0422, "Maono DM40 Live",     "DM40L.dll"),

    DM40G2(0x0425, "Maono DM40 Live Gen2"),
    DM40G2(0x0424, "Maono DM40 Live Gen2 Receiver"),
    DM40G2(0x0423, "Maono DM40 Live Gen2 Receiver"),
};

#undef MAONO_N

const MaonoDeviceInfo* MaonoFindDevice(unsigned short pid)
{
    for(unsigned int i = 0; i < MaonoDeviceCount(); i++)
    {
        if(maono_devices[i].pid == pid)
        {
            return(&maono_devices[i]);
        }
    }

    return(nullptr);
}

unsigned int MaonoDeviceCount()
{
    return((unsigned int)(sizeof(maono_devices) / sizeof(maono_devices[0])));
}

const MaonoDeviceInfo* MaonoDeviceAt(unsigned int index)
{
    return((index < MaonoDeviceCount()) ? &maono_devices[index] : nullptr);
}
