/*---------------------------------------------------------*\
| RGBController_HoltekF503.h                                |
|                                                           |
|   RGBController for Holtek mouse                          |
|                                                           |
|   Remigiusz Pietroń (remememe)                24 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "HoltekF503Controller.h"

class RGBController_HoltekF503 : public RGBController
{
public:
    RGBController_HoltekF503(HoltekF503Controller* controller_ptr);
    ~RGBController_HoltekF503();

    void            SetupZones();

    void            DeviceUpdateLEDs();
    void            DeviceUpdateZoneLEDs(int zone);
    void            DeviceUpdateSingleLED(int led);

    void            DeviceUpdateMode();

    unsigned char   GetHoltekModeFlag();

private:
    HoltekF503Controller* controller;
};
