/*---------------------------------------------------------*\
| RGBController_AcerBiFrostGPU.h                            |
|                                                           |
|   RGBController for Acer Predator BiFrost GPU             |
|                                                           |
|   Baobaosheng                                 18 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "AcerBiFrostGPUController.h"

enum
{
    ACER_BIFROST_MODE_DIRECT            = 0x00,
    ACER_BIFROST_MODE_OFF               = 0x01
};

class RGBController_AcerBiFrostGPU : public RGBController
{
public:
    RGBController_AcerBiFrostGPU(AcerBiFrostGPUController* controller_ptr);
    ~RGBController_AcerBiFrostGPU();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    AcerBiFrostGPUController*           controller;
};
