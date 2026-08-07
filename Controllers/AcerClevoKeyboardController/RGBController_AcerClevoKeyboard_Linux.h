/*---------------------------------------------------------*\
| RGBController_AcerClevoKeyboard_Linux.h                   |
|                                                           |
|   RGBController for Acer Clevo keyboard backlight         |
|   Uses TUXEDO driver sysfs interface                      |
|                                                           |
|   Shivam Purohit                               08 Aug 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "AcerClevoKeyboardController_Linux.h"

class RGBController_AcerClevoKeyboard : public RGBController
{
public:
    RGBController_AcerClevoKeyboard(AcerClevoKeyboardController* controller_ptr);
    ~RGBController_AcerClevoKeyboard();

    void SetupZones();

    void DeviceUpdateLEDs() override;
    void DeviceUpdateZoneLEDs(int zone) override;
    void DeviceUpdateSingleLED(int led) override;
    void DeviceUpdateMode() override;
    void DeviceSaveMode() override;

private:
    AcerClevoKeyboardController* controller;
};
