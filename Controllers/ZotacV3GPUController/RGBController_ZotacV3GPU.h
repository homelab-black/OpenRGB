/*---------------------------------------------------------*\
| RGBController_ZotacV3GPU.h                                |
|                                                           |
|   RGBController for Zotac V3 GPU                          |
|                                                           |
|   Based on RGBController_ZotacV2GPU                       |
|   Adapted for Zotac RTX 5070 SOLIC OC                     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "ZotacV3GPUController.h"

class RGBController_ZotacV3GPU : public RGBController
{
public:
    RGBController_ZotacV3GPU(ZotacV3GPUController* controller_ptr);
    ~RGBController_ZotacV3GPU();

    void SetupInitialValues();
    void SetupZones();

    void ResizeZone(int zone, int new_size);

    void DeviceUpdateLEDs();
    void UpdateZoneLEDs(int zone);
    void UpdateSingleLED(int led);

    void DeviceUpdateMode();

    ZotacV3GPUConfig config;

private:
    ZotacV3GPUController* controller;
};
