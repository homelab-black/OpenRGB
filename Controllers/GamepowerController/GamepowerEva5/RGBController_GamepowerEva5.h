/*---------------------------------------------------------*\
|   RGBController_GamepowerEva5.h                           |
|                                                           |
|   Driver for Gamepower Eva 5 keyboard                     |
|                                                           |
|   Made by FeusX  (23 Aug 2026)                            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/


#ifndef RGBCONTROLLER_GAMEPOWEREVA5_H
#define RGBCONTROLLER_GAMEPOWEREVA5_H

#include "RGBController.h"
#include "GamepowerEva5Controller.h"

class RGBController_GamepowerEva5 : public RGBController
{
public:
    RGBController_GamepowerEva5(GamepowerEva5Controller* controller_ptr);
    ~RGBController_GamepowerEva5();

    void SetupZones();

    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int /*zone*/);
    void DeviceUpdateSingleLED(int /*led*/);

    void DeviceUpdateMode();
private:
    GamepowerEva5Controller* controller;
};

#endif
