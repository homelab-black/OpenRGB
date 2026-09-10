/*---------------------------------------------------------*\
| RGBController_ASUSLampArray.cpp                           |
|                                                           |
|   RGBController for ASUS Vivobook LampArray Keyboard      |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_ASUSLampArray.h"

RGBController_ASUSLampArray::RGBController_ASUSLampArray(ASUSLampArrayController* controller_ptr, const std::string& dev_name)
{
    controller  = controller_ptr;

    name        = dev_name;
    vendor      = "ASUS";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "ASUS LampArray Keyboard Device";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();

    /*-----------------------------------------------------*\
    | Direct Mode (Software continuous streaming)           |
    \*-----------------------------------------------------*/
    mode Direct;
    Direct.name           = "Direct";
    Direct.value          = 0xFFFF;
    Direct.flags          = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Direct.brightness_min = 0;
    Direct.brightness_max = 100;
    Direct.brightness     = 100;
    Direct.color_mode     = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    /*-----------------------------------------------------*\
    | Static Mode (Fixed color, zero CPU/battery drain)     |
    \*-----------------------------------------------------*/
    mode Static;
    Static.name           = "Static";
    Static.value          = 0;
    Static.flags          = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Static.brightness_min = 0;
    Static.brightness_max = 100;
    Static.brightness     = 100;
    Static.color_mode     = MODE_COLORS_PER_LED;
    modes.push_back(Static);

    /*-----------------------------------------------------*\
    | Rainbow (Hardware / WMI EC - 0% CPU, 100% battery)    |
    \*-----------------------------------------------------*/
    mode Rainbow;
    Rainbow.name          = "Rainbow (Hardware)";
    Rainbow.value          = 1;
    Rainbow.flags          = 0;
    Rainbow.color_mode     = MODE_COLORS_NONE;
    modes.push_back(Rainbow);

    /*-----------------------------------------------------*\
    | Off Mode                                              |
    \*-----------------------------------------------------*/
    mode Off;
    Off.name              = "Off";
    Off.value             = 2;
    Off.flags             = 0;
    Off.color_mode        = MODE_COLORS_NONE;
    modes.push_back(Off);

    SetupZones();
}

RGBController_ASUSLampArray::~RGBController_ASUSLampArray()
{
    Shutdown();

    delete controller;
}

void RGBController_ASUSLampArray::SetupZones()
{
    zone keyboard_zone;
    keyboard_zone.name          = "Keyboard";
    keyboard_zone.type          = ZONE_TYPE_SINGLE;
    keyboard_zone.leds_min      = 1;
    keyboard_zone.leds_max      = 1;
    keyboard_zone.leds_count    = 1;
    zones.push_back(keyboard_zone);

    led keyboard_led;
    keyboard_led.name           = "Keyboard";
    leds.push_back(keyboard_led);

    SetupColors();
}

void RGBController_ASUSLampArray::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /* This device does not support resizing zones */
}

void RGBController_ASUSLampArray::DeviceUpdateLEDs()
{
    UpdateZoneLEDs(0);
}

void RGBController_ASUSLampArray::UpdateZoneLEDs(int /*zone*/)
{
    if(modes[active_mode].name == "Rainbow (Hardware)")
    {
        controller->SetHardwareRainbow(true);
        return;
    }
    else if(modes[active_mode].name == "Off")
    {
        controller->SetHardwareRainbow(false);
        controller->SetHostMode(true);
        controller->SetColor(0, 0, 0, 0);
        return;
    }

    controller->SetHardwareRainbow(false);
    controller->SetHostMode(true);

    unsigned char r = RGBGetRValue(colors[0]);
    unsigned char g = RGBGetGValue(colors[0]);
    unsigned char b = RGBGetBValue(colors[0]);

    unsigned char brightness_val = 255;
    if (modes[active_mode].flags & MODE_FLAG_HAS_BRIGHTNESS)
    {
        /* Map 0-100% to 0-255 intensity */
        brightness_val = (unsigned char)((modes[active_mode].brightness * 255) / 100);
    }

    controller->SetColor(r, g, b, brightness_val);
}

void RGBController_ASUSLampArray::UpdateSingleLED(int /*led*/)
{
    UpdateZoneLEDs(0);
}

void RGBController_ASUSLampArray::DeviceUpdateMode()
{
    UpdateZoneLEDs(0);
}
