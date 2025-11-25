/*---------------------------------------------------------*\
| RGBController_YunziiYZ75Keyboard.h                        |
|                                                           |
|   RGBController for Yunzii YZ75 keyboard                  |
|                                                           |
|   Kosta A. (kostaarvanitis)                   23 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/


#pragma once

#include "RGBController.h"
#include "YunziiYZ75KeyboardController.h"

class RGBController_YunziiYZ75Keyboard : public RGBController
{
public:
    RGBController_YunziiYZ75Keyboard(YunziiYZ75KeyboardController* controller_ptr);
    ~RGBController_YunziiYZ75Keyboard();

    void        SetupZones();
    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    YunziiYZ75KeyboardController* controller;
};
