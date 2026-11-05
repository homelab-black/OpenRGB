/*---------------------------------------------------------*\
| RGBController_RoyalKludgeS98.h                            |
|                                                           |
|   RGBController for Royal Kludge S98 keyboard             |
|                                                           |
|   manumyx                                     03 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "RGBController.h"
#include "RoyalKludgeS98Controller.h"

struct RoyalKludgeS98ModeUpdate
{
    unsigned char               mode_value;
    unsigned char               brightness;
    unsigned char               speed;
    unsigned char               color_flag;
    RGBColor                    color;
};

class RGBController_RoyalKludgeS98 : public RGBController
{
public:
    RGBController_RoyalKludgeS98(RoyalKludgeS98Controller* controller_ptr);
    ~RGBController_RoyalKludgeS98();

    void                        SetupZones();

    void                        DeviceUpdateLEDs();
    void                        DeviceUpdateZoneLEDs(int zone);
    void                        DeviceUpdateSingleLED(int led);

    void                        DeviceUpdateMode();

private:
    RoyalKludgeS98Controller*   controller;
    std::atomic<bool>           direct_active;

    /*-----------------------------------------------------*\
    | Frame writer thread                                   |
    |   DeviceUpdateLEDs() only queues the latest frame so  |
    |   the RGBController access lock is released quickly.  |
    |   The writer thread performs the paced USB transfers. |
    \*-----------------------------------------------------*/
    std::thread*                writer_thread;
    std::atomic<bool>           writer_thread_run;
    std::mutex                  writer_mutex;
    std::mutex                  frame_mutex;
    std::condition_variable     frame_cv;
    std::vector<RoyalKludgeS98LedSlot> frame_slots;
    bool                        frame_pending;
    bool                        frame_direct;
    unsigned int                frame_generation;
    std::atomic<unsigned int>   generation;
    bool                        mode_pending;
    RoyalKludgeS98ModeUpdate    pending_mode_update;

    void                        AddEffectMode(std::string mode_name, unsigned char mode_value, bool has_speed);
    void                        DiscardPendingFrame();
    void                        QueueFrame(const std::vector<RoyalKludgeS98LedSlot>& led_slots, bool direct);
    void                        QueueModeUpdate
                                    (
                                    unsigned char   mode_value,
                                    unsigned char   brightness,
                                    unsigned char   speed,
                                    unsigned char   color_flag,
                                    RGBColor        color
                                    );
    void                        WriterThreadFunction();
};
