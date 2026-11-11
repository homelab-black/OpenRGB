/*---------------------------------------------------------*\
| RGBController_AkkoMU02.h                                  |
|                                                           |
|   RGBController for Akko MU02 VIA keyboards               |
|                                                           |
|   OpenRGB contributors                        11 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "AkkoMU02Controller.h"

class RGBController_AkkoMU02 : public RGBController
{
public:
    RGBController_AkkoMU02(AkkoMU02Controller* controller_ptr);
    ~RGBController_AkkoMU02() override;

    void SetupZones();
    void DeviceUpdateMode() override;
    void DeviceSaveMode() override;
    void DeviceUpdateLEDs() override;
    void DeviceUpdateZoneLEDs(int zone) override;
    void DeviceUpdateSingleLED(int led) override;

private:
    AkkoMU02Controller* controller;
    RGBColor last_color;
    std::vector<RGBColor> last_key_colors;
    std::vector<bool> key_color_valid;
    unsigned char last_speed;
    unsigned char last_brightness;
};
