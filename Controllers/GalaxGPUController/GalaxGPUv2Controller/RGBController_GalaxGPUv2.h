/*---------------------------------------------------------*\
| RGBController_GalaxGPUv2.h                                |
|                                                           |
|   RGBController for GALAX/KFA2 RTX 30 Series GPU          |
|                                                           |
|   Muhamad Visat                               27 Jul 2025 |
|   Original work by Niels Westphal (crashniels)            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "GalaxGPUv2Controller.h"

class RGBController_GalaxGPUv2 : public RGBController
{
public:
    RGBController_GalaxGPUv2(GalaxGPUv2Controller* controller_ptr);
    ~RGBController_GalaxGPUv2();

    void SetupZones();

    void ResizeZone(int zone, int new_size);

    void DeviceUpdateLEDs();
    void UpdateZoneLEDs(int zone);
    void UpdateSingleLED(int led);

    void DeviceUpdateMode();
    void DeviceSaveMode();

private:
    GalaxGPUv2Controller* controller;
};
