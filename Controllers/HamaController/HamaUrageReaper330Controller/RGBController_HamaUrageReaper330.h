/*---------------------------------------------------------*\
| RGBController_HamaUrageReaper330.h                        |
|                                                           |
|   RGBController for Hama uRage Reaper 330 mouse           |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "HamaUrageReaper330Controller.h"

class RGBController_HamaUrageReaper330 : public RGBController
{
public:
    RGBController_HamaUrageReaper330(HamaUrageReaper330Controller* controller_ptr);
    ~RGBController_HamaUrageReaper330();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    HamaUrageReaper330Controller* controller;
};
