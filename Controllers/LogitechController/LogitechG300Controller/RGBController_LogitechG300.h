/*---------------------------------------------------------*\
| RGBController_LogitechG300.h                              |
|                                                           |
|   RGBController for Logitech G300s Optical Gaming Mouse   |
|                                                           |
|   Aman Ali                                  08 Sep 2026   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "LogitechG300Controller.h"

class RGBController_LogitechG300 : public RGBController
{
public:
    RGBController_LogitechG300(LogitechG300Controller* controller_ptr);
    ~RGBController_LogitechG300();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();
    void        DeviceSaveMode();

private:
    LogitechG300Controller* controller;
};
