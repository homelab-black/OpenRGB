/*---------------------------------------------------------*\
| RGBController_AntEsportsMK4100.h                          |
|                                                           |
| OpenRGB RGB controller for Ant Esports MK4100             |
| SPDX-License-Identifier: GPL-2.0-or-later                 |
\*---------------------------------------------------------*/

#pragma once

#include <cstdint>
#include <vector>
#include "RGBController.h"
#include "AntEsportsMK4100Controller.h"

class RGBController_AntEsportsMK4100 : public RGBController
{
public:
    explicit RGBController_AntEsportsMK4100(AntEsportsMK4100Controller* controller_ptr);
    ~RGBController_AntEsportsMK4100();

    void SetupZones();
    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);
    void DeviceUpdateMode();

private:
    void AddMode(const char* name, int value, bool has_brightness, bool has_speed,
                 bool has_color, int direction_type);
    void LoadCurrentDeviceState();
    bool BuildFrame(uint8_t rgb[MK4100_RGB_BYTES]) const;
    bool WriteSelectedMode();

    AntEsportsMK4100Controller* controller;
    int last_mode_index;
};
