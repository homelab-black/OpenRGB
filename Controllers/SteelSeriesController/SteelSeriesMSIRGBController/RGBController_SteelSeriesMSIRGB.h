/*---------------------------------------------------------*\
| RGBController_SteelSeriesMSIRGB.h                         |
|                                                           |
|   RGBController for MSI laptop SteelSeries RGB devices    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "SteelSeriesMSIRGBController.h"

class RGBController_SteelSeriesMSIRGB : public RGBController
{
public:
    RGBController_SteelSeriesMSIRGB(SteelSeriesMSIRGBController* controller_ptr);
    ~RGBController_SteelSeriesMSIRGB();

    void    SetupZones();

    void    ResizeZone(int zone, int new_size);

    void    DeviceUpdateLEDs();
    void    UpdateZoneLEDs(int zone);
    void    UpdateSingleLED(int led);

    void    DeviceUpdateMode();

private:
    SteelSeriesMSIRGBController* controller;
};
