/*-------------------------------------------------------------------*\
| RGBController_CMMM731Controller.h                                   |
|                                                                     |
|   Driver for the Cooler Master MM731 Mouse                          |
|                                                                     |
|   Chris M (Dr_No)                              1 Jan 2022           |
|                                                                     |
|   This file is part of the OpenRGB project                          |
|   SPDX-License-Identifier: GPL-2.0-only                             |
\*-------------------------------------------------------------------*/

#pragma once
#include "RGBController.h"
#include "CMMM731Controller.h"
#include <vector>

class RGBController_CMMM731Controller : public RGBController
{
public:
    RGBController_CMMM731Controller(CMMM731Controller *controller_ptr);
    ~RGBController_CMMM731Controller();

    void        SetupZones();
    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        SetCustomMode();
    void        DeviceUpdateMode();
    void        DeviceSaveMode();

private:
    void        Init_Controller();
    int         GetDeviceMode();
    int         GetLED_Zone(int led_idx);

    CMMM731Controller* controller;
};
