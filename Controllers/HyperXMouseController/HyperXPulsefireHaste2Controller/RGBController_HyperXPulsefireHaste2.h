/*---------------------------------------------------------*\
| RGBController_HyperXPulsefireHaste2.h                     |
|                                                           |
|   RGBController for HyperX Pulsefire Haste 2 Wireless     |
|                                                           |
|   Plamen Marinov (PMarinov1994)               14 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "HyperXPulsefireHaste2Controller.h"

class RGBController_HyperXPulsefireHaste2 : public RGBController
{
public:
    RGBController_HyperXPulsefireHaste2(HyperXPulsefireHaste2Controller* controller_ptr);
    ~RGBController_HyperXPulsefireHaste2();

    void SetupZones();

    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);

    void DeviceUpdateMode();
    void DeviceSaveMode();

private:
    HyperXPulsefireHaste2Controller*    controller;

    RGBColor GetActiveColor();
};
