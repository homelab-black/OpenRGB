/*---------------------------------------------------------*\
| RGBController_CreativePebbleX.h                           |
|                                                           |
|   RGBController for Creative Pebble X                    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "CreativePebbleXController.h"

#define PEBBLE_X_BRIGHTNESS_MIN 0x00
#define PEBBLE_X_BRIGHTNESS_MAX 0xFF
#define PEBBLE_X_SPEED_MIN      0x00
#define PEBBLE_X_SPEED_MAX      0x06

class RGBController_CreativePebbleX: public RGBController
{
public:
    RGBController_CreativePebbleX(CreativePebbleXController* controller_ptr);
    ~RGBController_CreativePebbleX();

    void        SetupZones();

    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    CreativePebbleXController*  controller;
};
