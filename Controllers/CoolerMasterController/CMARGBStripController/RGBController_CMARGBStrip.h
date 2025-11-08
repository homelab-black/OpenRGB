/*---------------------------------------------------------*\
| RGBController_CMARGBStrip.h                               |
|                                                           |
|   RGBController for Cooler Master ARGB light strip        |
|   devices                                                 |
|                                                           |
|   Morgan Guimard (morg)                       18 Sep 2023 |
|   Logan Phillips (Eclipse)                    05 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "CMARGBStripController.h"

class RGBController_CMARGBStrip : public RGBController
{
public:
    RGBController_CMARGBStrip(CMARGBStripController* controller_ptr, device_type dev_type);
    ~RGBController_CMARGBStrip();

    void SetupZones();

    void ResizeZone(int zone, int new_size);

    void DeviceUpdateLEDs();
    void UpdateZoneLEDs(int zone);
    void UpdateSingleLED(int led);

    void DeviceUpdateMode();

private:
    CMARGBStripController*  controller;
};
