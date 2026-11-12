/*---------------------------------------------------------*\
| RGBController_ColorfulARGB.h                               |
|                                                             |
|   RGBController for the onboard ARGB header on the         |
|   Colorful B650M Battle-AX Plus V14                         |
|                                                             |
|   This file is part of the OpenRGB project                 |
|   SPDX-License-Identifier: GPL-2.0-only                    |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "ColorfulARGBController.h"

class RGBController_ColorfulARGB : public RGBController
{
public:
    RGBController_ColorfulARGB(ColorfulARGBController* controller_ptr);
    ~RGBController_ColorfulARGB();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    ColorfulARGBController*    controller;
};
