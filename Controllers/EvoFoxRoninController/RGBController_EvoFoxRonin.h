/*---------------------------------------------------------*\
| RGBController_EvoFoxRonin.h                               |
|                                                           |
|   RGBController for EvoFox Ronin TKL RGB keyboard         |
|                                                           |
|   <Atif Usmani> (<AtifUsmani>)               <30 09 2026> |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "EvoFoxRoninController.h"

class RGBController_EvoFoxRonin : public RGBController
{
public:
    RGBController_EvoFoxRonin(EvoFoxRoninController* controller_ptr);
    ~RGBController_EvoFoxRonin();

    void SetupZones();
    void DeviceUpdateLEDs() override;
    void DeviceUpdateZoneLEDs(int zone) override;
    void DeviceUpdateSingleLED(int led) override;
    void DeviceUpdateMode() override;

private:
    EvoFoxRoninController* controller;
};
