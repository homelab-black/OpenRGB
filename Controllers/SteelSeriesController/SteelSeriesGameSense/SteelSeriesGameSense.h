/*---------------------------------------------------------*\
| SteelSeriesGameSense.h                                    |
|                                                           |
|   Sends 8-zone keyboard lighting through SteelSeries      |
|   Engine's local GameSense API. When GG/Engine is running |
|   (e.g. for Sonar) it overwrites raw HID writes, but it   |
|   displays GameSense events, so routing through it stops  |
|   the fight instead of racing it.                         |
|                                                           |
|   xmo4                                        12 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "RGBController.h"

/*---------------------------------------------------------*\
| Called from the keepalive thread when Engine goes away    |
| while it was showing our colors, so the owner can write   |
| the device directly again                                 |
\*---------------------------------------------------------*/
typedef void (*GameSenseLostCallback)(void*);

class SteelSeriesGameSense
{
public:
    SteelSeriesGameSense(GameSenseLostCallback callback, void* callback_arg);
    ~SteelSeriesGameSense();

    /*-----------------------------------------------------*\
    | Returns false when Engine is unavailable; the caller  |
    | then falls back to writing the device directly.       |
    \*-----------------------------------------------------*/
    bool        SendColors(const std::vector<RGBColor>& colors);

    /*-----------------------------------------------------*\
    | Hand lighting back to Engine (non-direct modes).      |
    \*-----------------------------------------------------*/
    void        Stop();

    /*-----------------------------------------------------*\
    | coreProps.json is in a shared ProgramData folder, so  |
    | only a 127.0.0.1 address is ever accepted from it.    |
    \*-----------------------------------------------------*/
    static bool ParseLoopbackAddress(const std::string& core_props, int& port);

private:
    bool        Register();
    bool        Post(const char* uri, const std::string& body);
    bool        SendLastColors();
    void        KeepaliveThreadFunction();

    GameSenseLostCallback                               lost_callback;
    void*                                               lost_callback_arg;
    std::mutex                                          mutex;
    std::vector<RGBColor>                               last_colors;
    std::chrono::time_point<std::chrono::steady_clock>  retry_after;
    int                                                 port;
    unsigned int                                        event_value;
    bool                                                registered;
    bool                                                active;
    std::atomic<bool>                                   keepalive_thread_run;
    std::thread                                         keepalive_thread;
};
