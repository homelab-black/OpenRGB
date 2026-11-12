/*---------------------------------------------------------*\
| RGBController_ColorfulARGB.cpp                             |
|                                                             |
|   This file is part of the OpenRGB project                 |
|   SPDX-License-Identifier: GPL-2.0-only                    |
\*---------------------------------------------------------*/

#include "RGBController_ColorfulARGB.h"

RGBController_ColorfulARGB::RGBController_ColorfulARGB(ColorfulARGBController* controller_ptr)
{
    controller  = controller_ptr;

    name        = "Colorful B650M Battle-AX Plus ARGB";
    vendor      = "Colorful";
    type        = DEVICE_TYPE_MOTHERBOARD;
    description = "Colorful B650M Battle-AX Plus onboard ARGB header";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();

    mode Direct;
    Direct.name       = "Direct";
    Direct.value      = 0;
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_ColorfulARGB::~RGBController_ColorfulARGB()
{
    Shutdown();
    delete controller;
}

void RGBController_ColorfulARGB::SetupZones()
{
    zone argb_zone;
    argb_zone.name       = "ARGB Header";
    argb_zone.type       = ZONE_TYPE_LINEAR;
    argb_zone.leds_min   = COLORFUL_ARGB_NUM_ZONES;
    argb_zone.leds_max   = COLORFUL_ARGB_NUM_ZONES;
    argb_zone.leds_count = COLORFUL_ARGB_NUM_ZONES;
    zones.push_back(argb_zone);

    for(unsigned int led_idx = 0; led_idx < COLORFUL_ARGB_NUM_ZONES; led_idx++)
    {
        led new_led;
        new_led.name = "ARGB Header LED " + std::to_string(led_idx + 1);
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_ColorfulARGB::DeviceUpdateLEDs()
{
    unsigned char zone_rgb[COLORFUL_ARGB_NUM_ZONES * 3];

    for(unsigned int led_idx = 0; led_idx < COLORFUL_ARGB_NUM_ZONES; led_idx++)
    {
        RGBColor color = colors[led_idx];
        zone_rgb[(led_idx * 3) + 0] = RGBGetRValue(color);
        zone_rgb[(led_idx * 3) + 1] = RGBGetGValue(color);
        zone_rgb[(led_idx * 3) + 2] = RGBGetBValue(color);
    }

    controller->SetColors(zone_rgb, COLORFUL_ARGB_NUM_ZONES);
}

void RGBController_ColorfulARGB::DeviceUpdateZoneLEDs(int /*zone*/)
{
    /* There's only 1 zone containing all LEDs, so updating the zone means updating everything */
    DeviceUpdateLEDs();
}

void RGBController_ColorfulARGB::DeviceUpdateSingleLED(int /*led*/)
{
    /* The device protocol always sends all 20 zones at once per cycle,
       there's no way to update a single LED -- so resend everything. */
    DeviceUpdateLEDs();
}

void RGBController_ColorfulARGB::DeviceUpdateMode()
{
    /* There's only a "Direct" mode (per-LED color), no other hardware
       mode that needs to be switched on the device. */
}
