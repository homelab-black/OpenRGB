/*---------------------------------------------------------*\
| RGBController_AcerClevoKeyboard_Linux.cpp                 |
|                                                           |
|   RGBController for Acer Clevo keyboard backlight         |
|   Uses TUXEDO driver sysfs interface                      |
|                                                           |
|   Shivam Purohit                               08 Aug 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_AcerClevoKeyboard_Linux.h"

RGBController_AcerClevoKeyboard::RGBController_AcerClevoKeyboard(AcerClevoKeyboardController* controller_ptr)
{
    controller = controller_ptr;

    name                    = controller->GetDeviceName();
    vendor                  = "Acer";
    type                    = DEVICE_TYPE_LAPTOP;
    description             = "Acer Clevo Keyboard Backlight";
    location                = controller->GetLocation();
    version                 = "1.0.0";

    mode static_mode;
    static_mode.name            = "Static";
    static_mode.value           = 0;
    static_mode.flags           = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    static_mode.color_mode      = MODE_COLORS_PER_LED;
    static_mode.speed_min       = 0;
    static_mode.speed_max       = 0;
    static_mode.speed           = 0;
    static_mode.brightness_min  = 0;
    static_mode.brightness_max  = 255;
    static_mode.brightness      = 255;
    modes.push_back(static_mode);

    SetupZones();
}

RGBController_AcerClevoKeyboard::~RGBController_AcerClevoKeyboard()
{
    Shutdown();

    delete controller;
}

void RGBController_AcerClevoKeyboard::SetupZones()
{
    unsigned int count = controller->GetZoneCount();

    zones.resize(1);
    zones[0].type       = (count <= 1) ? ZONE_TYPE_SINGLE : ZONE_TYPE_LINEAR;
    zones[0].name       = "Keyboard";
    zones[0].leds_min   = count;
    zones[0].leds_max   = count;
    zones[0].leds_count = count;

    leds.resize(count);

    for(unsigned int i = 0; i < count; i++)
    {
        leds[i].name = (count <= 1) ? "Keyboard Backlight" : ("Zone " + std::to_string(i + 1));
        leds[i].value = i;
    }

    SetupColors();
}

void RGBController_AcerClevoKeyboard::DeviceUpdateLEDs()
{
    if(colors.empty())
    {
        return;
    }

    RGBColor color = colors[0];
    unsigned char r = RGBGetRValue(color);
    unsigned char g = RGBGetGValue(color);
    unsigned char b = RGBGetBValue(color);

    controller->SetColor(r, g, b);
    controller->SetBrightness(modes[active_mode].brightness);
}

void RGBController_AcerClevoKeyboard::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_AcerClevoKeyboard::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_AcerClevoKeyboard::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}

void RGBController_AcerClevoKeyboard::DeviceSaveMode()
{
}
