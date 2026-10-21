/*---------------------------------------------------------*\
| RGBController_ChilkeyKeyboard.h                           |
|                                                           |
|   RGBController for Chilkey Slice75 HE keyboard           |
|                                                           |
|   Aaron Coyco                                 28 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "ChilkeyKeyboardController.h"

class RGBController_ChilkeyKeyboard : public RGBController
{
public:
    RGBController_ChilkeyKeyboard(ChilkeyKeyboardController* controller_ptr);
    ~RGBController_ChilkeyKeyboard();

    void SetupZones();

    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);

    void DeviceUpdateMode();

private:
    ChilkeyKeyboardController* controller;
};
