/*---------------------------------------------------------*\
| RGBController_ASUSLampArray.h                             |
|                                                           |
|   RGBController for ASUS Vivobook LampArray Keyboard      |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "ASUSLampArrayController.h"

class RGBController_ASUSLampArray : public RGBController
{
public:
    RGBController_ASUSLampArray(ASUSLampArrayController* controller_ptr, const std::string& dev_name = "ASUS Vivobook S 15 Keyboard");
    ~RGBController_ASUSLampArray();

    void        SetupZones();
    void        ResizeZone(int zone, int new_size);
    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);
    void        DeviceUpdateMode();

private:
    ASUSLampArrayController* controller;
};
