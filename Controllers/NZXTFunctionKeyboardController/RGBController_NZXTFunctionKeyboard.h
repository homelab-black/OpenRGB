/*---------------------------------------------------------*\
| RGBController_NZXTFunctionKeyboard.h                      |
|                                                           |
|   RGBController for NZXT Function RGB keyboard (ITE 8291) |
|                                                           |
|   P4tchW0rk (aekira@proton.me)          June 2026         |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "NZXTFunctionKeyboardController.h"
#include "NZXTFunctionKeyboardDevices.h"

class RGBController_NZXTFunctionKeyboard : public RGBController
{
public:
    RGBController_NZXTFunctionKeyboard(NZXTFunctionKeyboardController* controller_ptr);
    ~RGBController_NZXTFunctionKeyboard();

    void        SetupZones();

    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    NZXTFunctionKeyboardController*        controller;
    std::vector<RGBColor*>          buffer_map;
    RGBColor                        null_color;
};
