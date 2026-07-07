/*---------------------------------------------------------*\
| RGBController_ThermaltakeRiingToughFan.h                  |
|                                                           |
|   RGBController for Thermaltake ToughFan 14 RGB           |
|                                                           |
|   Omar Jaber (Bewinxed)                       18 Mar 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "ThermaltakeRiingToughFanController.h"

class RGBController_ThermaltakeRiingToughFan : public RGBController
{
public:
    RGBController_ThermaltakeRiingToughFan(ThermaltakeRiingToughFanController* controller_ptr);
    ~RGBController_ThermaltakeRiingToughFan();

    void        SetupZones();
    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    ThermaltakeRiingToughFanController* controller;
    std::vector<unsigned int>           leds_channel;
    std::vector<unsigned int>           zones_channel;
};
