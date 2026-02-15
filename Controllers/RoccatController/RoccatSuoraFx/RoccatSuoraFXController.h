/*---------------------------------------------------------*\
|  RoccatSuoraFXController.h                                |
|                                                           |
|  Driver for Roccat Suora FX Keyboard                      |
|                                                           |
|  AirPe                                         12 Feb 2026|
|                                                           |
|  This file is part of the OpenRGB project                 |
|  SPDX-License-Identifier: GPL-2.0-or-later                |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <hidapi/hidapi.h>

enum
{
    ROCCAT_SUORA_FX_EFFECT_FULLY_LIT    = 0x01,
    ROCCAT_SUORA_FX_EFFECT_BREATHING    = 0x02,
    ROCCAT_SUORA_FX_EFFECT_WAVE         = 0x03,
    ROCCAT_SUORA_FX_EFFECT_FADE         = 0x04,
    ROCCAT_SUORA_FX_EFFECT_SNAKE        = 0x05,
    ROCCAT_SUORA_FX_EFFECT_RIPPLE       = 0x06,
    ROCCAT_SUORA_FX_EFFECT_RAIN         = 0x0A,
    ROCCAT_SUORA_FX_EFFECT_GAME_OVER    = 0x09,
    ROCCAT_SUORA_FX_EFFECT_SCANNER      = 0x0C,
    ROCCAT_SUORA_FX_EFFECT_RADAR        = 0x0D,
    ROCCAT_SUORA_FX_EFFECT_DIRECT       = 0x10,
    ROCCAT_SUORA_FX_EFFECT_CUSTOM       = 0x33, // Used for Apply
};

// Effect Directions/Variants
enum
{
    ROCCAT_SUORA_FX_VARIANT_NONE        = 0x00,
    ROCCAT_SUORA_FX_VARIANT_RIGHT       = 0x01,
    ROCCAT_SUORA_FX_VARIANT_LEFT        = 0x02,
    ROCCAT_SUORA_FX_VARIANT_UP          = 0x03,
    ROCCAT_SUORA_FX_VARIANT_DOWN        = 0x04,
};

// Colors
enum
{
    ROCCAT_SUORA_FX_COLOR_RED           = 0x01,
    ROCCAT_SUORA_FX_COLOR_GREEN         = 0x02,
    ROCCAT_SUORA_FX_COLOR_YELLOW        = 0x03,
    ROCCAT_SUORA_FX_COLOR_BLUE          = 0x04,
    ROCCAT_SUORA_FX_COLOR_AQUA          = 0x05,
    ROCCAT_SUORA_FX_COLOR_PURPLE        = 0x06,
    ROCCAT_SUORA_FX_COLOR_WHITE         = 0x07,
};

class RoccatSuoraFXController
{
public:
    RoccatSuoraFXController(hid_device* dev_handle, const char* path);
    ~RoccatSuoraFXController();

    std::string GetDeviceLocation();
    
    void InitializeCustomMode();
    void SetEffect(unsigned char effect, unsigned char speed, unsigned char brightness, unsigned char color, unsigned char variant);
    void SendLightingData(unsigned char* colors);
    void SendKeyMap();
    void ApplyLighting();

private:
    hid_device*             handle;
    std::string             location;

    unsigned char           CalculateChecksum(unsigned char* data, int len);
    void                    SendControlReport(unsigned char* data, int len);
    void                    SendInterruptPacket(unsigned char* data, int len);
};
