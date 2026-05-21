/*---------------------------------------------------------*\
| RGBController_AulaF2067.cpp                               |
|                                                           |
|   RGBController for Aula Aegis 2067usb                    |
|   (a.k.a F2067) keyboard                                  |
|                                                           |
|   Antipov Kostiantyn (dev@diablosat.cc)      21.05.2026   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_AulaF2067.h"

struct effect_element
{
    std::string   name;
    unsigned char val;
};

RGBController_AulaF2067::RGBController_AulaF2067(AulaF2067Controller* controller_ptr)
{
    controller  = controller_ptr;

    name        = "Aula F2067";
    vendor      = "Aula";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "Aula F2067 Keyboard Device";
    location    = controller->GetDeviceLocation();

    modes.clear();

    effect_element effect_list[] =
    {
        {"Swim", 0x00},
        {"Gossip / Sea", 0x01},
        {"Rainbow wheel", 0x02},
        {"Ripple diffusion", 0x03},
        {"Retinue scanning", 0x04},
        {"Crossfire", 0x05},
        {"Respire", 0x06},
        {"Blossoming", 0x07},
        {"Shadow follows", 0x08},
        {"Comet tail-tracking", 0x09},
        {"Rotating windmill", 0x0a},
        {"Fixed_on", 0x0b},
        {"Stars_twinkle", 0x0c},
        {"Neon stream", 0x0d},
        {"Instant", 0x0e},
        {"Progressive scanning", 0x0f},
        {"Collision", 0x10},
        {"Rotating storm", 0x11},
        {"Random flying", 0x12},
        {"Colorful waterfalls", 0x13},
    };

    for(unsigned int i = 0; i < sizeof(effect_list) / sizeof(effect_list[0]); i++)
    {
        mode new_mode;
        new_mode.name       = effect_list[i].name;
        new_mode.value      = effect_list[i].val;

        new_mode.flags      = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
        new_mode.color_mode = MODE_COLORS_NONE;

        new_mode.brightness_min = AULA_F2067_BRIGHTNESS_MIN;
        new_mode.brightness_max = AULA_F2067_BRIGHTNESS_MAX;
        new_mode.brightness     = AULA_F2067_BRIGHTNESS;

        new_mode.speed_min      = AULA_F2067_SPEED_MIN;
        new_mode.speed_max      = AULA_F2067_SPEED_MAX;
        new_mode.speed          = AULA_F2067_SPEED;

        modes.push_back(new_mode);
    }

    active_mode = 0;
    SetupZones();
}

void RGBController_AulaF2067::SetupZones()
{
    /*---------------------------------------------------------*\
    | Set up zone                                               |
    \*---------------------------------------------------------*/
    zones.resize(1);
    zones[0].type           = ZONE_TYPE_SINGLE;
    zones[0].name           = "Keyboard Backlight zone";
    zones[0].leds_min       = 1;
    zones[0].leds_max       = 1;
    zones[0].leds_count     = 1;
    zones[0].matrix_map     = NULL;

    /*---------------------------------------------------------*\
    | Set up LED                                                |
    \*---------------------------------------------------------*/
    leds.resize(1);
    leds[0].name = "Keyboard Backlight LED";

    SetupColors();
}

void RGBController_AulaF2067::DeviceUpdateMode()
{
    controller->setAnimationEffect(modes[active_mode].value);
    controller->setBrightness(modes[active_mode].brightness);
    controller->setSpeed(modes[active_mode].speed);

    controller->SendPacket();
}

void RGBController_AulaF2067::DeviceUpdateLEDs()
{
    DeviceUpdateMode();
}

void RGBController_AulaF2067::ResizeZone(int, int)
{
    // This device does not support resizing zones
}

void RGBController_AulaF2067::UpdateZoneLEDs(int)
{
    DeviceUpdateLEDs();
}

void RGBController_AulaF2067::UpdateSingleLED(int)
{
    DeviceUpdateLEDs();
}

RGBController_AulaF2067::~RGBController_AulaF2067()
{
    delete controller;
}