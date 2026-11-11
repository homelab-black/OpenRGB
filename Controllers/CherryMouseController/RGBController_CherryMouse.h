/*---------------------------------------------------------*\
| RGBController_CherryMouse.h                               |
|                                                           |
|   RGBController for Cherry mouse                          |
|                                                           |
|   Rik Vanden Boer                             03 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "CherryMouseController.h"

enum
{
    CHERRY_MOUSE_MODE_SPECTRUM                      = 0x00,
    CHERRY_MOUSE_MODE_BREATHING                     = 0x01,
    CHERRY_MOUSE_MODE_STATIC                        = 0x02,
};

class RGBController_CherryMouse : public RGBController
{
public:
    RGBController_CherryMouse(CherryMouseController* controller_ptr);
    ~RGBController_CherryMouse();

    void SetupZones();

    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);

    void DeviceUpdateMode();

private:
    CherryMouseController* controller;
};
