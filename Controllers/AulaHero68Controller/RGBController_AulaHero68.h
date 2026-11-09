/*---------------------------------------------------------*\
| RGBController_AulaHero68.h                                |
|                                                           |
|   RGBController for AULA Hero 68 keyboard                 |
|                                                           |
|   Original author: dunn1o, 2026-08-09                     |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

#include "AulaHero68Controller.h"
#include "RGBController.h"

class RGBController_AulaHero68 : public RGBController
{
public:
    RGBController_AulaHero68(AulaHero68Controller* controller_ptr);
    ~RGBController_AulaHero68();

    void SetupZones();
    void ResizeZone(int zone, int new_size);

    void DeviceUpdateLEDs();
    void UpdateZoneLEDs(int zone);
    void UpdateSingleLED(int led);
    void DeviceUpdateMode();

private:
    AulaHero68Controller* controller;
    std::vector<RGBColor> current_colors;
    bool                  direct_mode_initialized;
    std::chrono::steady_clock::time_point last_direct_frame_time;
    std::chrono::steady_clock::time_point last_bulk_signature_time;
    uint64_t              last_bulk_signature;
};
