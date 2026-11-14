/*---------------------------------------------------------*\
| RGBController_SamsungSSD_Windows_Linux.h                  |
|                                                           |
|   RGBController for Samsung SSD heatsink LED              |
|                                                           |
|   Volodymyr Kotyk (harley4ik)                 04 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "SamsungSSDController_Windows_Linux.h"

class RGBController_SamsungSSD : public RGBController
{
public:
    RGBController_SamsungSSD(SamsungSSDController* controller_ptr);
    ~RGBController_SamsungSSD();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    SamsungSSDController*   controller;
};
