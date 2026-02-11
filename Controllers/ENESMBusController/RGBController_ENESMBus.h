/*---------------------------------------------------------*\
| RGBController_ENESMBus.h                                  |
|                                                           |
|   RGBController for ENE SMBus devices                     |
|                                                           |
|   Adam Honse (CalcProgrammer1)                13 Jun 2019 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "ENESMBusController.h"
#include <nlohmann/json.hpp>

#define ENESMBUS_CFG_KEY "ENESMBusSettings"

struct ENESMBusSettings
{
    bool enable_save;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(ENESMBusSettings,
    enable_save
);

class RGBController_ENESMBus : public RGBController
{
public:
    RGBController_ENESMBus(ENESMBusController* controller_ptr);
    ~RGBController_ENESMBus();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();
    void        DeviceSaveMode();

private:
    ENESMBusController* controller;

    int         GetDeviceMode();
};
