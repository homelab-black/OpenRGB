/*---------------------------------------------------------*\
| RGBController_Maono.h                                     |
|                                                           |
|   RGBController for Maono microphones                     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "MaonoController.h"
#include "RGBController.h"

enum
{
    MAONO_MODE_DIRECT       = 0,    /* per-LED colour (exact or nearest preset)  */
    MAONO_MODE_BREATHING    = 1,
    MAONO_MODE_COLOR_CYCLE  = 2,
    MAONO_MODE_RHYTHM       = 3,
    MAONO_MODE_OFF          = 4,
    MAONO_MODE_SPECIAL      = 0x100,/* + index into MaonoDeviceInfo::specials    */
};

class RGBController_Maono : public RGBController
{
public:
    RGBController_Maono(MaonoController* controller_ptr);
    ~RGBController_Maono();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();
    void        DeviceSaveMode();

    static const MaonoColorPreset*  NearestPreset(RGBColor color, const MaonoColorPreset* presets, unsigned int count);

private:
    MaonoController*        controller;
    const MaonoDeviceInfo*  info;

    void        ApplyStaticColor(unsigned int channel, RGBColor color);
    void        ApplyPulseColor(unsigned int channel, RGBColor color);
};
