/*---------------------------------------------------------*\
| RGBController_AVerMediaGC570D_Windows.h                   |
|                                                           |
|   RGBController for AVerMedia Live Gamer DUO (GC570D)     |
|                                                           |
|   Niaz (derneuere)                             24 Aug 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "AVerMediaGC570DController_Windows.h"

class RGBController_AVerMediaGC570D: public RGBController
{
public:
    RGBController_AVerMediaGC570D(AVerMediaGC570DController* controller_ptr);
    ~RGBController_AVerMediaGC570D();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    AVerMediaGC570DController*  controller;
};
