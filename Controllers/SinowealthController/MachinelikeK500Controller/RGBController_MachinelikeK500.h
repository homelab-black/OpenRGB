/*---------------------------------------------------------*\
| RGBController_MachinelikeK500.h                           |
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
    void        ResizeZone(int zone, int new_size);

    void        DeviceUpdateLEDs();
    void        UpdateZoneLEDs(int zone);
    void        UpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    MachinelikeK500Controller* controller;
    std::function<void()> shutdown_callback;
};
