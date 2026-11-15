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

    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);

    void DeviceUpdateMode();

private:
    Zotac4AGPUController* controller;

    void SetupInitialValues();
};
