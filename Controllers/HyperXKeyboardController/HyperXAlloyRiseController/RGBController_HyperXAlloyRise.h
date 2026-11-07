/*---------------------------------------------------------*\
| RGBController_HyperXAlloyRise.h                           |
|                                                           |
|   RGBController for HyperX Alloy Rise keyboard            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <atomic>
#include <chrono>
#include <thread>
#include "RGBController.h"
#include "HyperXAlloyRiseController.h"

class RGBController_HyperXAlloyRise : public RGBController
{
public:
    RGBController_HyperXAlloyRise(HyperXAlloyRiseController* controller_ptr);
    ~RGBController_HyperXAlloyRise();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

    void        KeepaliveThread();

private:
    HyperXAlloyRiseController*                          controller;
    std::thread*                                        keepalive_thread;
    std::atomic<bool>                                   keepalive_thread_run;
    std::chrono::time_point<std::chrono::steady_clock>  last_update_time;
};
