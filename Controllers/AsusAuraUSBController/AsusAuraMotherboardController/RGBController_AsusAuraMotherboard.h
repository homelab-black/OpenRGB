/*---------------------------------------------------------*\
| RGBController_AsusAuraMotherboard.h                           |
|                                                           |
|   RGBController for ASUS Aura motherboard                     |
|                                                           |
|   Mola19                                      08 Mar 2022 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "AsusAuraMotherboardController.h"

class RGBController_AuraMotherboard : public RGBController
{
public:
    RGBController_AuraMotherboard(AuraMotherboardController* controller_ptr);
    ~RGBController_AuraMotherboard();

    void        SetupZones();
    void        SetupModes();
    void        DeviceConfigureZone(int zone_idx);
    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();
    void        DeviceSaveMode();

private:
    AuraMotherboardController*  controller;
};
