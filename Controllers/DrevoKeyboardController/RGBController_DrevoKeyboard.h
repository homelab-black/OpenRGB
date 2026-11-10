/*---------------------------------------------------------*\
| RGBController_DrevoKeyboard.h                             |
|                                                           |
|   RGBController for Drevo keyboards                       |
|                                                           |
|   Dennis Blokland                             02 Jan 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "DrevoKeyboardController.h"
#include "RGBController.h"

class RGBController_DrevoKeyboard : public RGBController
{
public:
    RGBController_DrevoKeyboard(DrevoKeyboardController* controller_ptr);
    ~RGBController_DrevoKeyboard();

    void        SetupZones();
    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    DrevoKeyboardController*    controller;
};
