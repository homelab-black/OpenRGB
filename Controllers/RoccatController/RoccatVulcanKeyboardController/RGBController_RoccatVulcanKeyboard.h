/*---------------------------------------------------------*\
| RGBController_RoccatVulcanKeyboard.h                      |
|                                                           |
|   RGBController for Roccat Vulcan keyboard                |
|                                                           |
|   Mola19                                      17 Dec 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include "RGBController.h"
#include "RoccatVulcanKeyboardController.h"

class RGBController_RoccatVulcanKeyboard : public RGBController
{
public:
    RGBController_RoccatVulcanKeyboard(RoccatVulcanKeyboardController* controller_ptr);
    ~RGBController_RoccatVulcanKeyboard();

    void        SetupZones();

    void        DeviceUpdateLEDs();
    void        DeviceUpdateZoneLEDs(int zone);
    void        DeviceUpdateSingleLED(int led);

    void        DeviceUpdateMode();

private:
    RoccatVulcanKeyboardController* controller;
    uint16_t                    pid;

    static RGBColor Scale5009(RGBColor color, unsigned int brightness);
    static RGBColor Hue5009(double hue);
    void EffectLoop();
    std::thread effect_thread;
    std::atomic<bool> effect_stop{false};
    std::atomic<unsigned int> effect_mode{0};
    std::atomic<unsigned int> effect_speed{ROCCAT_VULCAN_SPEED_DEFAULT};
    std::atomic<unsigned int> effect_brightness{ROCCAT_VULCAN_BRIGHTNESS_DEFAULT};
    std::mutex effect_io_mutex;
    std::mutex effect_data_mutex;
    std::mutex effect_wait_mutex;
    std::condition_variable effect_cv;
    std::vector<led_color> effect_base_colors;
    std::vector<unsigned int> effect_led_ids;
};
