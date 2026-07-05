/*---------------------------------------------------------*\
| RGBController_LogitechG110.cpp                            |
|                                                           |
|   RGBController for Logitech G110 Gaming Keyboard         |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "RGBController_LogitechG110.h"

static const char* zone_names[] =
{
    "Keyboard"
};

static zone_type zone_types[] =
{
    ZONE_TYPE_SINGLE
};

static const unsigned int zone_sizes[] =
{
    1
};

static const char* led_names[] =
{
    "Keyboard"
};

/**------------------------------------------------------------------*\
    @name Logitech G110
    @category Keyboard
    @type USB
    @save :robot:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectLogitechKeyboardG110
    @comment
\*-------------------------------------------------------------------*/

RGBController_LogitechG110::RGBController_LogitechG110(LogitechG110Controller* controller_ptr)
{
    controller                  = controller_ptr;

    name                        = "Logitech G110 Gaming Keyboard";
    vendor                      = "Logitech";
    type                        = DEVICE_TYPE_KEYBOARD;
    description                 = "Logitech G110 Gaming Keyboard";
    location                    = controller->GetDeviceLocation();
    serial                      = controller->GetSerialString();

    mode Direct;
    Direct.name                 = "Direct";
    Direct.value                = 0xFFFF;
    Direct.flags                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Direct.color_mode           = MODE_COLORS_PER_LED;
    Direct.brightness_min       = 0;
    Direct.brightness_max       = 255;
    Direct.brightness           = 255;
    modes.push_back(Direct);

    mode Static;
    Static.name                 = "Static";
    Static.value                = LOGITECH_G110_MODE_STATIC;
    Static.flags                = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Static.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    Static.colors_min           = 1;
    Static.colors_max           = 1;
    Static.colors.resize(1);
    Static.brightness_min       = 0;
    Static.brightness_max       = 255;
    Static.brightness           = 255;
    modes.push_back(Static);

    SetupZones();
}

RGBController_LogitechG110::~RGBController_LogitechG110()
{
    delete controller;
}

void RGBController_LogitechG110::SetupZones()
{
    /*---------------------------------------------------------*\
    | Set up zones                                              |
    \*---------------------------------------------------------*/
    unsigned int total_led_count = 0;

    for(unsigned int zone_idx = 0; zone_idx < 1; zone_idx++)
    {
        zone new_zone;
        new_zone.name           = zone_names[zone_idx];
        new_zone.type           = zone_types[zone_idx];
        new_zone.leds_min       = zone_sizes[zone_idx];
        new_zone.leds_max       = zone_sizes[zone_idx];
        new_zone.leds_count     = zone_sizes[zone_idx];
        new_zone.matrix_map     = NULL;

        zones.push_back(new_zone);

        total_led_count        += zone_sizes[zone_idx];
    }

    for(unsigned int led_idx = 0; led_idx < total_led_count; led_idx++)
    {
        led new_led;
        new_led.name            = led_names[led_idx];
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_LogitechG110::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_LogitechG110::DeviceUpdateLEDs()
{
    /*---------------------------------------------------------*\
    | Update all LEDs by updating the single zone               |
    \*---------------------------------------------------------*/
    if(active_mode == 0)
    {
        UpdateZoneLEDs(0);
    }
}

void RGBController_LogitechG110::UpdateZoneLEDs(int zone)
{
    /*---------------------------------------------------------*\
    | The G110 sets the whole keyboard to a single color        |
    \*---------------------------------------------------------*/
    if(active_mode == 0 && zones[zone].leds_count > 0)
    {
        RGBColor      color      = zones[zone].colors[0];
        unsigned char brightness = modes[active_mode].brightness;
        controller->SetColor(RGBGetRValue(color), RGBGetBValue(color), brightness);
    }
}

void RGBController_LogitechG110::UpdateSingleLED(int led)
{
    /*---------------------------------------------------------*\
    | The G110 sets the whole keyboard to a single color        |
    \*---------------------------------------------------------*/
    if(active_mode == 0)
    {
        RGBColor      color      = colors[led];
        unsigned char brightness = modes[active_mode].brightness;
        controller->SetColor(RGBGetRValue(color), RGBGetBValue(color), brightness);
    }
}

void RGBController_LogitechG110::DeviceUpdateMode()
{
    /*---------------------------------------------------------*\
    | Send the color to the G110 device based on the mode       |
    \*---------------------------------------------------------*/
    if(active_mode != 0)
    {
        RGBColor color = 0;

        if(modes[active_mode].color_mode == MODE_COLORS_MODE_SPECIFIC)
        {
            color = modes[active_mode].colors[0];
        }

        unsigned char brightness = modes[active_mode].brightness;
        controller->SetColor(RGBGetRValue(color), RGBGetBValue(color), brightness);
    }
}
