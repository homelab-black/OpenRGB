/*---------------------------------------------------------*\
| RGBController_SkydimoSerial.h                            |
|                                                           |
|   RGBController for Skydimo serial devices               |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "SkydimoSerialController.h"

class RGBController_SkydimoSerial : public RGBController
{
public:
    RGBController_SkydimoSerial(SkydimoSerialController* controller_ptr);
    ~RGBController_SkydimoSerial();

    void        SetupZones();
    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);
    void        DeviceUpdateMode();

private:
    void        ClearZoneData();

    SkydimoSerialController* controller;
};
