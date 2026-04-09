/*---------------------------------------------------------*\
| RGBController_Zotac4AGPU.h                                |
|                                                           |
|   RGBController for Zotac 4A GPU                          |
|                                                           |
|   Peter Berendi                               29 Dec 2024 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "Zotac4AGPUController.h"

class RGBController_Zotac4AGPU : public RGBController
{
public:
    RGBController_Zotac4AGPU(Zotac4AGPUController* controller_ptr);
    ~RGBController_Zotac4AGPU();

    void SetupZones();

    void ResizeZone(int zone, int new_size);

    void DeviceUpdateLEDs();
    void UpdateZoneLEDs(int zone);
    void UpdateSingleLED(int led);

    void DeviceUpdateMode();

private:
    Zotac4AGPUController* controller;

    std::chrono::time_point<std::chrono::steady_clock> lastupdatetime; // when the last packet was sent to the controller

    void SetupInitialValues();
};
