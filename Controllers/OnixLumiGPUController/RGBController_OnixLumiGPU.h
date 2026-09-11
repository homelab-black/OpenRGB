/*---------------------------------------------------------*\
| RGBController_OnixLumiGPU.h                               |
|                                                           |
|   RGBController for ONIX LUMI Intel Arc B580              |
|                                                           |
|   Jan Dvořák                                  11 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "OnixLumiGPUController.h"

class RGBController_OnixLumiGPU : public RGBController
{
public:
    RGBController_OnixLumiGPU(OnixLumiGPUController* controller_ptr);
    ~RGBController_OnixLumiGPU();

    void SetupZones();

    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);

    void DeviceUpdateMode();

private:
    OnixLumiGPUController* controller;
};
