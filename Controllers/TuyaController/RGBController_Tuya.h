/*---------------------------------------------------------*\
| RGBController_Tuya.h                                       |
|                                                             |
|   RGBController for Tuya-based WiFi RGB lights              |
|                                                             |
|   This file is part of the OpenRGB project                 |
|   SPDX-License-Identifier: GPL-2.0-or-later                |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "TuyaController.h"

class RGBController_Tuya : public RGBController
{
public:
    RGBController_Tuya(TuyaController* controller_ptr);
    ~RGBController_Tuya();

    void        SetupZones();
    void        DeviceConfigureZone(int zone_idx);

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    TuyaController* controller;
};
