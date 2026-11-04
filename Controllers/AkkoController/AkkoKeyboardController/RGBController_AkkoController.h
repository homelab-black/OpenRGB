/*---------------------------------------------------------*\
| RGBController_Akko5075BController.h                       |
|                                                           |
|   RGBController for Akko 5075B keyboard                   |
|                                                           |
|   Bizzotto Nicola (nicolaa02nb)               05 Feb 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "AkkoController.h"
#include <map>

class RGBController_AkkoController : public RGBController
{
public:
    RGBController_AkkoController(AkkoController *controller_ptr);
    ~RGBController_AkkoController();
    
    void SetupZones();
    void ResizeZone(int zone, int new_size);
    
    void DeviceUpdateLEDs();
    void UpdateZoneLEDs(int zone);
    void UpdateSingleLED(int led);
    
    void DeviceUpdateMode();
    void DeviceSaveMode();

private:
    AkkoController* controller;

    std::map<AKKO_ZONE, AkkoZoneInfo> zones_info;

    AKKO_FLAGS GetFlagsForMode(AKKO_MODE mode, unsigned int direction, unsigned int random_colors);
    unsigned int GetColorModeFromFlags(AKKO_MODE mode, AKKO_FLAGS flags);
    unsigned int GetDirectionFromFlags(AKKO_FLAGS flags);
};
