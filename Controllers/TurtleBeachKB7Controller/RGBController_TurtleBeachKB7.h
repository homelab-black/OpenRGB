/*---------------------------------------------------------*\
| RGBController_TurtleBeachKB7.h                            |
|                                                           |
|   RGBController for Turtle Beach Command Series KB7       |
|                                                           |
|   Joe Peterson (joevpeterson@gmail.com)       04 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "TurtleBeachKB7Controller.h"

enum
{
    TURTLE_BEACH_KB7_MODE_DIRECT
};

class RGBController_TurtleBeachKB7 : public RGBController
{
public:
    RGBController_TurtleBeachKB7(TurtleBeachKB7Controller* controller_ptr);
    ~RGBController_TurtleBeachKB7();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    TurtleBeachKB7Controller*   controller;
};
