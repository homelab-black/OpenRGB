/*---------------------------------------------------------*\
| RGBController_MachinelikeK500.h                           |
|                                                           |
|   This is a version of SinowealthKeyboardController       |
|   Modified to support Machinelike K500 keyboards          |
|                                                           |
|   Lizeth Villar (xllser)                      30 Mar 2025 |
|   Dmitry Kychanov (k1-801)                    18 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "RGBController.h"
#include "MachinelikeK500Controller.h"

class RGBController_MachinelikeK500 : public RGBController
{
public:
    RGBController_MachinelikeK500(MachinelikeK500Controller* controller_ptr, std::function<void()> shutdown_callback);
    ~RGBController_MachinelikeK500();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    MachinelikeK500Controller* controller;
    std::function<void()> shutdown_callback;
};
