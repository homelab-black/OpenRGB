/*---------------------------------------------------------*\
| RGBController_RoyalKludgeH81.h                            |
|                                                           |
|   RGBController for Royal Kludge RK-H81 keyboard          |
|                                                           |
|   eclecticBelletrist                          04 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include "RGBController.h"
#include "RoyalKludgeH81Controller.h"

namespace rkh81
{
    enum
    {
        MODE_OFF    = 0x0,
        MODE_DIRECT = 0x1,
    };
}

class RGBController_RoyalKludgeH81 : public RGBController
{
public:
    RGBController_RoyalKludgeH81(RoyalKludgeH81Controller* controller_ptr);
    ~RGBController_RoyalKludgeH81();

    RGBController_RoyalKludgeH81(const RGBController_RoyalKludgeH81&)            = delete;
    RGBController_RoyalKludgeH81& operator=(const RGBController_RoyalKludgeH81&) = delete;

    void SetupZones();

    void DeviceUpdateLEDs();
    void DeviceUpdateZoneLEDs(int zone);
    void DeviceUpdateSingleLED(int led);

    void DeviceUpdateMode();

private:
    void KeepaliveThreadFunction();

    // nanoseconds since epoch (steady_clock), atomic since KeepaliveThreadFunction
    // reads it off the calling thread with no other synchronization
    std::atomic<int64_t>       last_update_time_ns;
    RoyalKludgeH81Controller*  controller;
    std::atomic<bool>          keepalive_thread_run;
    std::thread*               keepalive_thread;
    std::mutex                 keepalive_wake_mutex;
    std::condition_variable    keepalive_wake_cv;
};
