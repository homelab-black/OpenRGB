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
    bool        ReinitDirectMode();

private:
    LogitechHIDPPController*    controller;

    /*---------------------------------------------------------*\
    | Maps OpenRGB LED index -> HID++ per-key zone ID           |
    \*---------------------------------------------------------*/
    std::vector<uint16_t>       led_to_zone_id;

    /*---------------------------------------------------------*\
    | Reverse map: zone_id -> LED index (-1 if no LED).         |
    | Indexed 0..255 (zone IDs are bytes). Built once in        |
    | SetupZones to avoid scanning led_to_zone_id at commit     |
    | time, which would be O(N) per acked zone.                 |
    \*---------------------------------------------------------*/
    std::vector<int>            zone_id_to_led_idx;

    /*---------------------------------------------------------*\
    | Last successfully committed colors for delta updates.     |
    | An entry of HIDPP_UNCOMMITTED (0xFF000000) marks an LED   |
    | whose last write didn't ACK and which therefore needs to  |
    | be re-pushed in the next frame regardless of color delta. |
    | The high byte (0xFF) is impossible for any value produced |
    | by ToRGBColor() so it never collides with a real color.   |
    \*---------------------------------------------------------*/
    std::vector<RGBColor>       sent_colors;
    uint32_t                    last_init_gen = 0;
};
