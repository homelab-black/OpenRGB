/*---------------------------------------------------------*\
| RGBController_EVisionV3Keyboard.h                         |
|                                                           |
|   RGBController for EVision "RDR" tri-mode keyboards      |
|                                                           |
|   Jan Imrich (imrija)                         05 Jul 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "EVisionV3KeyboardController.h"

class RGBController_EVisionV3Keyboard : public RGBController
{
public:
    RGBController_EVisionV3Keyboard(EVisionV3KeyboardController* controller_ptr);
    ~RGBController_EVisionV3Keyboard();

    void        SetupZones();
    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();
    void        DeviceSaveMode();

private:
    EVisionV3KeyboardController* controller;

    void        ReadDeviceState();
};
