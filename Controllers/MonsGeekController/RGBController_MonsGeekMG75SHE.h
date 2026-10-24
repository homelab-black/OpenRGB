/*---------------------------------------------------------*\
| RGBController_MonsGeekMG75SHE.h                           |
|                                                           |
|   RGBController for the MonsGeek MG75S HE keyboard        |
|                                                           |
|   Sergey Tsai (mustang651)                 14 Sep 2026    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <mutex>
#include "MonsGeekMG75SHEController.h"
#include "RGBController.h"

class RGBController_MonsGeekMG75SHE : public RGBController
{
public:
    RGBController_MonsGeekMG75SHE(MonsGeekMG75SHEController* controller);
    ~RGBController_MonsGeekMG75SHE();

    void SetupZones();

    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);
    void DeviceUpdateMode();

private:
    void AddEffect(const char* effect_name, int effect_value, unsigned int effect_flags, unsigned int default_direction = MODE_DIRECTION_RIGHT);
    bool EnsureStaticMode();

    MonsGeekMG75SHEController* controller;
    bool static_mode_prepared;
    std::mutex device_mutex;
};
