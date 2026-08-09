/*---------------------------------------------------------*\
| RGBController_RedragonK652.h                             |
|                                                          |
|   RGBController for the wired Redragon K652               |
|                                                          |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "RedragonK652Controller.h"

class RGBController_RedragonK652 : public RGBController
{
public:
    RGBController_RedragonK652(RedragonK652Controller* controller_ptr);
    ~RGBController_RedragonK652();

    void SetupZones();

    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);
    void DeviceUpdateMode();
    void DeviceEnableUserLightingControl();

private:
    RedragonK652Controller* controller;
};
