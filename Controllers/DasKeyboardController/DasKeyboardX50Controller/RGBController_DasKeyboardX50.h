/*---------------------------------------------------------*\
| RGBController_DasKeyboardX50.h                            |
|                                                           |
|   RGBController for the Das Keyboard X50 RGB             |
|                                                           |
|   Sergio Presa                                15 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "DasKeyboardX50Controller.h"

class RGBController_DasKeyboardX50 : public RGBController
{
public:
    RGBController_DasKeyboardX50(DasKeyboardX50Controller* controller_ptr);
    ~RGBController_DasKeyboardX50();
    void SetupZones();
    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);
    void DeviceUpdateMode();
private:
    DasKeyboardX50Controller* controller;
};
