/*---------------------------------------------------------*\
|  RGBController_SinowealthKeyboard.h                       |
|                                                           |
|  Definitions and types for Sinowealth                     |
|  Keyboard, Hopefully generic, this was                    |
|  made spefically for FL eSports F11 KB                    |
|                                                           |
|  Dmitri Kalinichenko (Dima-Kal) 23/06/2021                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "SinowealthKeyboardController.h"

class RGBController_SinowealthKeyboard : public RGBController
{
public:
    RGBController_SinowealthKeyboard(SinowealthKeyboardController* controller_ptr, std::function<void()> shutdown_callback);
    ~RGBController_SinowealthKeyboard();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    SinowealthKeyboardController* controller;
    std::function<void()>         shutdown_callback;
};
