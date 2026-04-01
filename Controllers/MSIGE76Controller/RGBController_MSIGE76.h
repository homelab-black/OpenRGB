/*---------------------------------------------------------*\
| RGBController_MSIGE76.h                                   |
|                                                           |
|   RGBController for MSI GE76 Raider KLC/ALC              |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "MSIGE76Controller.h"

class RGBController_MSIGE76 : public RGBController
{
public:
    RGBController_MSIGE76(MSIGE76Controller* controller_ptr);
    ~RGBController_MSIGE76();

    void    SetupZones();
    void    ResizeZone(int zone, int new_size);
    void    DeviceUpdateLEDs();
    void    UpdateZoneLEDs(int zone);
    void    UpdateSingleLED(int led);
    void    DeviceUpdateMode();

private:
    MSIGE76Controller*  controller;
};
