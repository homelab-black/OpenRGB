/*---------------------------------------------------------*\
| RGBController_CorsairHydroH100i.h                         |
|                                                           |
|   RGBController for Corsair H100i Hydro Series cooler     |
|                                                           |
|   RaddedMC (mycooljames@hotmail.com)          27 Jun 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "CorsairHydroH100iController.h"

class RGBController_CorsairHydroH100i : public RGBController
{
public:
    RGBController_CorsairHydroH100i(CorsairHydroH100iController* controller_ptr);
    ~RGBController_CorsairHydroH100i();

    void        SetupZones();

    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    CorsairHydroH100iController* controller;
};
