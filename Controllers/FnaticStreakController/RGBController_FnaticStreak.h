/*---------------------------------------------------------*\
| RGBController_FnaticStreak.h                              |
|                                                           |
|   RGBController for Fnatic Streak keyboard                |
|                                                           |
|   Based on leddy by Hanna Czenczek                        |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "FnaticStreakController.h"

class RGBController_FnaticStreak : public RGBController
{
public:
    RGBController_FnaticStreak(FnaticStreakController* controller_ptr);
    ~RGBController_FnaticStreak();

    void        SetupZones();

    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    FnaticStreakController* controller;
};
