
/*---------------------------------------------------------*\
|   RGBController_ThermaltakeArgentMP1.h                    |
|                                                           |
|   OpenRGB UI Wrapper for Thermaltake Argent MP1 Mousepad  |
|                                                           |
|   John Scripter                                1 Nov 2026 |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "ThermaltakeArgentMP1Controller.h"

class RGBController_ThermaltakeArgentMP1 : public RGBController
{
public:
    RGBController_ThermaltakeArgentMP1(ThermaltakeArgentMP1Controller* controller_ptr);
    ~RGBController_ThermaltakeArgentMP1();

    void SetupZones();
    void ResizeZone(int zone, int new_size);
    void DeviceUpdateLEDs();
    void UpdateZoneLEDs(int zone);
    void UpdateSingleLED(int led);

    void DeviceUpdateMode();

private:
    ThermaltakeArgentMP1Controller* controller;
};
