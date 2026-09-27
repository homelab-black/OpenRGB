/*---------------------------------------------------------*\
| RGBController_DrevoKeyboard.h                             |
|                                                           |
|   RGBController for DREVO BladeMaster PRO keyboards       |
|                                                           |
|   T-bond                                      27 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "DrevoKeyboardController.h"

class RGBController_DrevoKeyboard : public RGBController
{
public:
    RGBController_DrevoKeyboard(DrevoKeyboardController* controller_ptr);
    ~RGBController_DrevoKeyboard();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    DrevoKeyboardController*    controller;
};
