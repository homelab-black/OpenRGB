/*---------------------------------------------------------*\
| RGBController_Yeelight.cpp                                |
|                                                           |
|   RGBController for Yeelight                              |
|                                                           |
|   Adam Honse (CalcProgrammer1)                18 Jan 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_Yeelight.h"

/**------------------------------------------------------------------*\
    @name Yeelight
    @category Light
    @type Network
    @save :x:
    @direct :rotating_light:
    @effects :white_check_mark:
    @detectors DetectYeelightControllers
    @comment
\*-------------------------------------------------------------------*/

RGBController_Yeelight::RGBController_Yeelight(YeelightController* controller_ptr)
{
    controller  = controller_ptr;

    name        = controller->GetName();
    vendor      = controller->GetManufacturer();
    type        = DEVICE_TYPE_LIGHT;
    version     = controller->GetVersion();
    description = "Yeelight Device";
    serial      = controller->GetUniqueID();
    location    = controller->GetLocation();

    mode Direct;
    Direct.name       = controller->GetProtocol() == YEELIGHT_PROTOCOL_TCP ? "Static" : "Direct";
    Direct.value      = 0;
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    if(controller->GetProtocol() == YEELIGHT_PROTOCOL_UDP)
    {
        mode White2000K;
        White2000K.name       = "White (2000K)";
        White2000K.value      = YEELIGHT_MODE_WHITE2000K;
        White2000K.flags      = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
        White2000K.color_mode = MODE_COLORS_PER_LED;
        White2000K.brightness     = YEELIGHT_BRIGHTNESS_MAX;
        White2000K.brightness_min = YEELIGHT_BRIGHTNESS_MIN;
        White2000K.brightness_max = YEELIGHT_BRIGHTNESS_MAX;
        modes.push_back(White2000K);

        mode White3000K;
        White3000K.name       = "White (3000K)";
        White3000K.value      = YEELIGHT_MODE_WHITE3000K;
        White3000K.flags      = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
        White3000K.color_mode = MODE_COLORS_PER_LED;
        White3000K.brightness     = YEELIGHT_BRIGHTNESS_MAX;
        White3000K.brightness_min = YEELIGHT_BRIGHTNESS_MIN;
        White3000K.brightness_max = YEELIGHT_BRIGHTNESS_MAX;
        modes.push_back(White3000K);

        mode White4000K;
        White4000K.name       = "White (4000K)";
        White4000K.value      = YEELIGHT_MODE_WHITE4000K;
        White4000K.flags      = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
        White4000K.color_mode = MODE_COLORS_PER_LED;
        White4000K.brightness     = YEELIGHT_BRIGHTNESS_MAX;
        White4000K.brightness_min = YEELIGHT_BRIGHTNESS_MIN;
        White4000K.brightness_max = YEELIGHT_BRIGHTNESS_MAX;
        modes.push_back(White4000K);

        mode White5000K;
        White5000K.name       = "White (5000K)";
        White5000K.value      = YEELIGHT_MODE_WHITE5000K;
        White5000K.flags      = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
        White5000K.color_mode = MODE_COLORS_PER_LED;
        White5000K.brightness     = YEELIGHT_BRIGHTNESS_MAX;
        White5000K.brightness_min = YEELIGHT_BRIGHTNESS_MIN;
        White5000K.brightness_max = YEELIGHT_BRIGHTNESS_MAX;
        modes.push_back(White5000K);

        mode White6500K;
        White6500K.name       = "White (6500K)";
        White6500K.value      = YEELIGHT_MODE_WHITE6500K;
        White6500K.flags      = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
        White6500K.color_mode = MODE_COLORS_PER_LED;
        White6500K.brightness     = YEELIGHT_BRIGHTNESS_MAX;
        White6500K.brightness_min = YEELIGHT_BRIGHTNESS_MIN;
        White6500K.brightness_max = YEELIGHT_BRIGHTNESS_MAX;
        modes.push_back(White6500K);
    }

    SetupZones();
}

RGBController_Yeelight::~RGBController_Yeelight()
{
    Shutdown();

    delete controller;
}

void RGBController_Yeelight::SetupZones()
{
    zone led_zone;
    led_zone.name       = "RGB Light";
    led_zone.type       = ZONE_TYPE_SINGLE;
    led_zone.leds_min   = 1;
    led_zone.leds_max   = 1;
    led_zone.leds_count = 1;
    zones.push_back(led_zone);

    led new_led;
    new_led.name = "RGB Light";

    leds.push_back(new_led);

    SetupColors();
}

void RGBController_Yeelight::DeviceUpdateLEDs()
{
    unsigned char red = RGBGetRValue(colors[0]);
    unsigned char grn = RGBGetGValue(colors[0]);
    unsigned char blu = RGBGetBValue(colors[0]);

    controller->SetColor(red, grn, blu);
}

void RGBController_Yeelight::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_Yeelight::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_Yeelight::DeviceUpdateMode()
{
    if (modes[active_mode].value != YEELIGHT_MODE_DIRECT)
    {
        int temperature = 0;
        switch (modes[active_mode].value)
        {
        case YEELIGHT_MODE_WHITE2000K: temperature = 2000; break;
        case YEELIGHT_MODE_WHITE3000K: temperature = 3000; break;
        case YEELIGHT_MODE_WHITE4000K: temperature = 4000; break;
        case YEELIGHT_MODE_WHITE5000K: temperature = 5000; break;
        case YEELIGHT_MODE_WHITE6500K: temperature = 6500; break;
        default: break;
        }
        controller->SetColorTemperature(temperature, modes[active_mode].brightness);
    }
}
