/*---------------------------------------------------------*\
| RGBController_GMK67.h                                     |
|                                                           |
|   RGBController for the GMK67 keyboard                    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "GMK67Controller.h"

class RGBController_GMK67 : public RGBController
{
public:
    RGBController_GMK67(GMK67Controller* controller_ptr);
    ~RGBController_GMK67();

    void SetupZones();

    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);

    void DeviceUpdateMode();

private:
    GMK67Controller* controller;

    void SendFrame();
};
