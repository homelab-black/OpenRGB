/*---------------------------------------------------------*\
| RGBController_AkkoKeyboard.h                               |
|                                                           |
|   RGBController for Akko / RongYuan keyboards.            |
|                                                           |
|   Per-key addressing is not available on stock firmware,  |
|   so each of the two hardware zones (keys, side bars) is   |
|   modelled as a single-LED zone (one color per zone).     |
|                                                           |
|   Roz                                         20 Jun 2026 |
|                                                           |
|   Protocol by the echtzeit-solutions/monsgeek-akko-linux  |
|   project (docs/PROTOCOL.md) and the SonixQMK community.  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "AkkoKeyboardController.h"

class RGBController_AkkoKeyboard : public RGBController
{
public:
    RGBController_AkkoKeyboard(AkkoKeyboardController* controller_ptr);
    ~RGBController_AkkoKeyboard();

    void        SetupZones();
    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    AkkoKeyboardController*  controller;

    void        SetupModes();
    void        SendZone(int zone);
};
