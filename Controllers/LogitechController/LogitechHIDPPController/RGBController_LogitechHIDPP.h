/*---------------------------------------------------------*\
| RGBController_LogitechHIDPP.h                             |
|                                                           |
|   RGBController for unified Logitech HID++ 2.0 devices   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "LogitechHIDPPController.h"

class RGBController_LogitechHIDPP : public RGBController
{
public:
    RGBController_LogitechHIDPP(LogitechHIDPPController* controller_ptr);
    ~RGBController_LogitechHIDPP();

    void        SetupZones();
    void        ResizeZone(int zone, int new_size);
    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);
    void        DeviceUpdateMode();

private:
    LogitechHIDPPController*    controller;

    /*---------------------------------------------------------*\
    | Maps OpenRGB LED index -> HID++ per-key zone ID           |
    \*---------------------------------------------------------*/
    std::vector<uint16_t>       led_to_zone_id;
};
