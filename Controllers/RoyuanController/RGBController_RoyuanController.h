/*---------------------------------------------------------*\
| RGBController_RoyuanController.h                          |
|                                                           |
|   RGBController for Royuan keyboard                       |
|                                                           |
|   Alvaro Munoz (alvaromunoz)                  05 Jun 2023 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "RoyuanController.h"

class RGBController_RoyuanController : public RGBController
{
public:
    RGBController_RoyuanController(RoyuanController* controller_ptr);
    virtual ~RGBController_RoyuanController();

    void        SetupZones();
    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    RoyuanController* controller;
};
