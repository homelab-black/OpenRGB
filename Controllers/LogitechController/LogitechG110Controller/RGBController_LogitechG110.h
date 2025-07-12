/*---------------------------------------------------------*\
| RGBController_LogitechG110.h                              |
|                                                           |
|   RGBController for Logitech G110 Gaming Keyboard         |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "LogitechG110Controller.h"

class RGBController_LogitechG110 : public RGBController
{
public:
    RGBController_LogitechG110(LogitechG110Controller* controller_ptr);
    ~RGBController_LogitechG110();

    void        SetupZones();

    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    LogitechG110Controller* controller;
};
