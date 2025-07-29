/*---------------------------------------------------------*\
| RGBController_GalaxGPUv3.h                                |
|                                                           |
|   RGBController for Galax GPUs (Xtreme Tuner)             |
|                                                           |
|   Daniel Stuart  (daniel.stuart14)            26 may 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "GalaxGPUv3Controller.h"

class RGBController_GalaxGPUv3 : public RGBController
{
public:
    RGBController_GalaxGPUv3(GalaxGPUv3Controller* controller_ptr);
    ~RGBController_GalaxGPUv3();

    void        SetupZones();

    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();
    void        DeviceSaveMode();

private:
    GalaxGPUv3Controller* controller;

    int        GetDeviceMode();
};
