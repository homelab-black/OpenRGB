/*---------------------------------------------------------*\
|   RGBController_GamepowerEva5.cpp                         |
|                                                           |
|   Driver for Gamepower Eva 5 keyboard                     |
|                                                           |
|   Made by FeusX  (23 Aug 2026)                            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/


#include "RGBController_GamepowerEva5.h"

static const char* eva5_mode_names[] =
{
    "Steady",
    "Special Key",
    "Breathing",
    "Press and Destroy",
    "Neon Stream",
    "Streamer",
    "Flowing Light and Color",
    "Dripping Ripples",
    "Brilliant Point",
    "Flash Away",
    "Shadow Disappear",
    "Ripples Shining",
    "Rich and Honoured",
    "Marquee Effect",
    "Rotating Storm",
    "Serpentine Horse Race",
    "Stars Twinkle",
    "Retro Snake",
    "Diagonal Transformation",
    "Sine Wave",
};

RGBController_GamepowerEva5::RGBController_GamepowerEva5(GamepowerEva5Controller* controller_ptr)
{
    controller  = controller_ptr;
    name        = "Gamepower Eva5";
    vendor      = "Gamepower";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "Gamepower Eva5 Keyboard";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();

    mode Mode;
    for(unsigned int i = 0; i < (sizeof(eva5_mode_names) / sizeof(eva5_mode_names[0])); i++)
    {
        Mode.name           = eva5_mode_names[i];
        Mode.value          = i;
        Mode.flags          = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
        Mode.color_mode     = MODE_COLORS_NONE;
        Mode.colors_min     = 0;
        Mode.colors_max     = 0;
        Mode.brightness_min = GAMEPOWER_EVA5_BRIG_LVL0;
        Mode.brightness_max = GAMEPOWER_EVA5_BRIG_LVL5;
        Mode.brightness     = GAMEPOWER_EVA5_BRIG_LVL5;
        Mode.speed_min      = GAMEPOWER_EVA5_SPEED_LVL0;
        Mode.speed_max      = GAMEPOWER_EVA5_SPEED_LVL2;
        Mode.speed          = GAMEPOWER_EVA5_SPEED_LVL2;
        modes.push_back(Mode);
    }

    SetupZones();

}

RGBController_GamepowerEva5::~RGBController_GamepowerEva5()
{
    delete controller;
}

void RGBController_GamepowerEva5::SetupZones()
{
    /* Eva5 has no per-key colour control, led colours are immutable */
    zone Zone;
    Zone.name           = "Keyboard";
    Zone.type           = ZONE_TYPE_SINGLE;
    Zone.leds_min       = 1;
    Zone.leds_max       = 1;
    Zone.leds_count     = 1;
    zones.push_back(Zone);

    led Led;
    Led.name = "Keyboard";
    leds.push_back(Led);

    SetupColors();
}

void RGBController_GamepowerEva5::DeviceUpdateMode()
{
    controller->SetMode(modes[active_mode].value, modes[active_mode].brightness, modes[active_mode].speed);
}

void RGBController_GamepowerEva5::DeviceUpdateLEDs(){}

void RGBController_GamepowerEva5::DeviceUpdateZoneLEDs(int /*zone*/){}

void RGBController_GamepowerEva5::DeviceUpdateSingleLED(int /*led*/){}
