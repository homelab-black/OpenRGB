/*---------------------------------------------------------*\
| RGBController_TOPCRGB.h                                   |
|                                                           |
|   RGBController for TOPC RGB devices                      |
|                                                           |
|   Diam (last.monoliths@gmail.com)              6 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "TOPCRGBController.h"

class RGBController_TOPCRGB : public RGBController
{
public:
    RGBController_TOPCRGB(TOPCRGBController* controller_ptr);
    ~RGBController_TOPCRGB();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);
    void        DeviceUpdateMode();

private:
    TOPCRGBController* controller;
};

