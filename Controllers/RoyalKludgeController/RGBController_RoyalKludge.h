/*---------------------------------------------------------*\
| RGBController_RoyalKludge.h                               |
|                                                           |
|   RGBController for Royal Kludge RK987 keyboards          |
|                                                           |
|   johnm                                       01 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "RoyalKludgeController.h"

class RGBController_RoyalKludge : public RGBController
{
public:
    RGBController_RoyalKludge(RoyalKludgeController* controller_ptr);
    ~RGBController_RoyalKludge();

    void                SetupZones();

    void                DeviceUpdateLEDs();
    void                DeviceUpdateZoneLEDs(int zone);
    void                DeviceUpdateSingleLED(int led);

    void                DeviceUpdateMode();

private:
    RoyalKludgeController*      controller;
    std::vector<unsigned int>   led_columns;

    void                DeviceUpdateRainbow();
};
