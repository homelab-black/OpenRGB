/* Das Keyboard X50 RGBController - SPDX-License-Identifier: GPL-2.0-or-later */
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
