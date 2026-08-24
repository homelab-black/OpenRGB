/*---------------------------------------------------------*\
| AVerMediaGC570DController_Windows.h                       |
|                                                           |
|   Driver for AVerMedia Live Gamer DUO (GC570D) (Windows)  |
|                                                           |
|   Niaz (derneuere)                             24 Aug 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <windows.h>

#define AVERMEDIA_GC570D_MODE_OFF                   0xF0
#define AVERMEDIA_GC570D_MODE_STATIC                0xF1
#define AVERMEDIA_GC570D_MODE_BREATHING             0xF2
#define AVERMEDIA_GC570D_MODE_RAINBOW_BREATHING     0xF3
#define AVERMEDIA_GC570D_MODE_BLINKING              0xF4
#define AVERMEDIA_GC570D_MODE_RAINBOW_BLINKING      0xF5
#define AVERMEDIA_GC570D_MODE_MARQUEE               0xF6

#define AVERMEDIA_GC570D_SPEED_SLOWEST              0x07
#define AVERMEDIA_GC570D_SPEED_FASTEST              0x00
#define AVERMEDIA_GC570D_SPEED_DEFAULT              0x04

#define AVERMEDIA_GC570D_BRIGHTNESS_MIN             0
#define AVERMEDIA_GC570D_BRIGHTNESS_MAX             100

class AVerMediaGC570DController
{
public:
    AVerMediaGC570DController();
    ~AVerMediaGC570DController();

    bool            Initialize();

    std::string     GetDeviceName();
    std::string     GetDeviceLocation();

    void            SetColor(unsigned char red, unsigned char green, unsigned char blue, unsigned char speed);
    void            SetMode(unsigned char pattern);

private:
    HANDLE          device_handle;
    std::string     device_path;

    bool            SetProperty(unsigned int property_id, const unsigned char* payload);
    bool            GetProperty(unsigned int property_id, unsigned char* payload);
    bool            PropertySupported(unsigned int property_id);
};
