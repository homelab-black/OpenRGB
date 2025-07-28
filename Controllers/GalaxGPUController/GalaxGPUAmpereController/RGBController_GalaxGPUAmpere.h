/*---------------------------------------------------------*\
| RGBController_GalaxGPUAmpere.h                            |
|                                                           |
|   RGBController for Galax/KFA2 GPU                        |
|                                                           |
|   Muhamad Visat                               27 Jul 2025 |
|   Original work by Niels Westphal (crashniels)            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "GalaxGPUAmpereController.h"

class RGBController_GalaxGPUAmpere : public RGBController
{
public:
    RGBController_GalaxGPUAmpere(GalaxGPUAmpereController* controller_ptr);
    ~RGBController_GalaxGPUAmpere();

    void SetupZones();

    void ResizeZone(int zone, int new_size);

    void DeviceUpdateLEDs();
    void UpdateZoneLEDs(int zone);
    void UpdateSingleLED(int led);

    void DeviceUpdateMode();
    void DeviceSaveMode();

private:
    GalaxGPUAmpereController* controller;
};
