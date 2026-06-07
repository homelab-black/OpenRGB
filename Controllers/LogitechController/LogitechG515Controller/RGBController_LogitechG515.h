/*---------------------------------------------------------*\
| RGBController_LogitechG515.h                              |
|                                                           |
|   RGBController for Logitech G515 LIGHTSPEED TKL         |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "LogitechG515Controller.h"
#include "RGBController.h"

class RGBController_LogitechG515 : public RGBController
{
public:
    RGBController_LogitechG515(LogitechG515Controller* controller_ptr);
    ~RGBController_LogitechG515();

    void        SetupZones();

    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    LogitechG515Controller* controller;
};
