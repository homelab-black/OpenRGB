/*---------------------------------------------------------*\
| RGBController_MCHOSEV9Dock.h                              |
|                                                           |
|   RGBController for MCHOSE V9 charging dock               |
|                                                           |
|   Michael Mano <michael.mano26@gmail.com>     24 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <chrono>
#include "RGBController.h"
#include "MCHOSEV9DockController.h"

class RGBController_MCHOSEV9Dock : public RGBController
{
public:
    RGBController_MCHOSEV9Dock(MCHOSEV9DockController* controller_ptr);
    ~RGBController_MCHOSEV9Dock();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

    void        KeepaliveThread();

private:
    MCHOSEV9DockController*                             controller;
    std::thread*                                        keepalive_thread;
    std::atomic<bool>                                   keepalive_thread_run;
    std::chrono::time_point<std::chrono::steady_clock>  last_update_time;
};
