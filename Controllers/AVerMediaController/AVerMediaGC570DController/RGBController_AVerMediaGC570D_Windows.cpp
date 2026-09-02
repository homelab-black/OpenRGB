/*---------------------------------------------------------*\
| RGBController_AVerMediaGC570D_Windows.cpp                 |
|                                                           |
|   RGBController for AVerMedia Live Gamer DUO (GC570D)     |
|                                                           |
|   Niaz (derneuere)                             24 Aug 2026|
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_AVerMediaGC570D_Windows.h"

/**------------------------------------------------------------------*\
    @name AVerMedia Live Gamer DUO
    @category Capture Card
    @type PCI
    @save :x:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectAVerMediaGC570DControllers
    @comment Lighting is exposed through a vendor KS property set on the
        capture filter. Colour writes are staged by the driver and only
        reach the hardware when the pattern property is written, so every
        update writes the colour first and the pattern second. The card
        has no brightness register, so brightness is applied by scaling
        the colour before it is sent, matching what the vendor software
        does. The rainbow modes generate their own colours in firmware
        and therefore expose neither colour nor brightness.
\*-------------------------------------------------------------------*/

RGBController_AVerMediaGC570D::RGBController_AVerMediaGC570D(AVerMediaGC570DController* controller_ptr)
{
    controller                  = controller_ptr;

    name                        = controller->GetDeviceName();
    vendor                      = "AVerMedia";
    type                        = DEVICE_TYPE_ACCESSORY;
    description                 = "AVerMedia Live Gamer DUO (GC570D)";
    location                    = controller->GetDeviceLocation();

    mode Direct;
    Direct.name                 = "Direct";
    Direct.value                = AVERMEDIA_GC570D_MODE_STATIC;
    Direct.flags                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Direct.color_mode           = MODE_COLORS_PER_LED;
    Direct.brightness_min       = AVERMEDIA_GC570D_BRIGHTNESS_MIN;
    Direct.brightness_max       = AVERMEDIA_GC570D_BRIGHTNESS_MAX;
    Direct.brightness           = AVERMEDIA_GC570D_BRIGHTNESS_MAX;
    modes.push_back(Direct);

    mode Static;
    Static.name                 = "Static";
    Static.value                = AVERMEDIA_GC570D_MODE_STATIC;
    Static.flags                = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Static.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    Static.colors_min           = 1;
    Static.colors_max           = 1;
    Static.colors.resize(1);
    Static.brightness_min       = AVERMEDIA_GC570D_BRIGHTNESS_MIN;
    Static.brightness_max       = AVERMEDIA_GC570D_BRIGHTNESS_MAX;
    Static.brightness           = AVERMEDIA_GC570D_BRIGHTNESS_MAX;
    modes.push_back(Static);

    mode Breathing;
    Breathing.name              = "Breathing";
    Breathing.value             = AVERMEDIA_GC570D_MODE_BREATHING;
    Breathing.flags             = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
    Breathing.color_mode        = MODE_COLORS_MODE_SPECIFIC;
    Breathing.colors_min        = 1;
    Breathing.colors_max        = 1;
    Breathing.colors.resize(1);
    Breathing.brightness_min    = AVERMEDIA_GC570D_BRIGHTNESS_MIN;
    Breathing.brightness_max    = AVERMEDIA_GC570D_BRIGHTNESS_MAX;
    Breathing.brightness        = AVERMEDIA_GC570D_BRIGHTNESS_MAX;
    Breathing.speed_min         = AVERMEDIA_GC570D_SPEED_SLOWEST;
    Breathing.speed_max         = AVERMEDIA_GC570D_SPEED_FASTEST;
    Breathing.speed             = AVERMEDIA_GC570D_SPEED_DEFAULT;
    modes.push_back(Breathing);

    mode Blinking;
    Blinking.name               = "Blinking";
    Blinking.value              = AVERMEDIA_GC570D_MODE_BLINKING;
    Blinking.flags              = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
    Blinking.color_mode         = MODE_COLORS_MODE_SPECIFIC;
    Blinking.colors_min         = 1;
    Blinking.colors_max         = 1;
    Blinking.colors.resize(1);
    Blinking.brightness_min     = AVERMEDIA_GC570D_BRIGHTNESS_MIN;
    Blinking.brightness_max     = AVERMEDIA_GC570D_BRIGHTNESS_MAX;
    Blinking.brightness         = AVERMEDIA_GC570D_BRIGHTNESS_MAX;
    Blinking.speed_min          = AVERMEDIA_GC570D_SPEED_SLOWEST;
    Blinking.speed_max          = AVERMEDIA_GC570D_SPEED_FASTEST;
    Blinking.speed              = AVERMEDIA_GC570D_SPEED_DEFAULT;
    modes.push_back(Blinking);

    mode RainbowBreathing;
    RainbowBreathing.name       = "Rainbow Breathing";
    RainbowBreathing.value      = AVERMEDIA_GC570D_MODE_RAINBOW_BREATHING;
    RainbowBreathing.flags      = MODE_FLAG_HAS_SPEED;
    RainbowBreathing.color_mode = MODE_COLORS_NONE;
    RainbowBreathing.speed_min  = AVERMEDIA_GC570D_SPEED_SLOWEST;
    RainbowBreathing.speed_max  = AVERMEDIA_GC570D_SPEED_FASTEST;
    RainbowBreathing.speed      = AVERMEDIA_GC570D_SPEED_DEFAULT;
    modes.push_back(RainbowBreathing);

    mode RainbowBlinking;
    RainbowBlinking.name        = "Rainbow Blinking";
    RainbowBlinking.value       = AVERMEDIA_GC570D_MODE_RAINBOW_BLINKING;
    RainbowBlinking.flags       = MODE_FLAG_HAS_SPEED;
    RainbowBlinking.color_mode  = MODE_COLORS_NONE;
    RainbowBlinking.speed_min   = AVERMEDIA_GC570D_SPEED_SLOWEST;
    RainbowBlinking.speed_max   = AVERMEDIA_GC570D_SPEED_FASTEST;
    RainbowBlinking.speed       = AVERMEDIA_GC570D_SPEED_DEFAULT;
    modes.push_back(RainbowBlinking);

    mode Marquee;
    Marquee.name                = "Marquee";
    Marquee.value               = AVERMEDIA_GC570D_MODE_MARQUEE;
    Marquee.flags               = MODE_FLAG_HAS_SPEED;
    Marquee.color_mode          = MODE_COLORS_NONE;
    Marquee.speed_min           = AVERMEDIA_GC570D_SPEED_SLOWEST;
    Marquee.speed_max           = AVERMEDIA_GC570D_SPEED_FASTEST;
    Marquee.speed               = AVERMEDIA_GC570D_SPEED_DEFAULT;
    modes.push_back(Marquee);

    mode Off;
    Off.name                    = "Off";
    Off.value                   = AVERMEDIA_GC570D_MODE_OFF;
    Off.flags                   = 0;
    Off.color_mode              = MODE_COLORS_NONE;
    modes.push_back(Off);

    SetupZones();
}

RGBController_AVerMediaGC570D::~RGBController_AVerMediaGC570D()
{
    Shutdown();

    delete controller;
}

void RGBController_AVerMediaGC570D::SetupZones()
{
    zone led_zone;

    led_zone.name               = "Board";
    led_zone.type               = ZONE_TYPE_SINGLE;
    led_zone.leds_min           = 1;
    led_zone.leds_max           = 1;
    led_zone.leds_count         = 1;
    zones.push_back(led_zone);

    led new_led;

    new_led.name                = "Board LED";
    leds.push_back(new_led);

    SetupColors();
}

void RGBController_AVerMediaGC570D::DeviceUpdateLEDs()
{
    unsigned char   red;
    unsigned char   green;
    unsigned char   blue;
    unsigned char   speed       = (unsigned char)modes[active_mode].speed;
    unsigned int    brightness  = modes[active_mode].brightness;

    if(modes[active_mode].color_mode == MODE_COLORS_MODE_SPECIFIC)
    {
        red                     = RGBGetRValue(modes[active_mode].colors[0]);
        green                   = RGBGetGValue(modes[active_mode].colors[0]);
        blue                    = RGBGetBValue(modes[active_mode].colors[0]);
    }
    else
    {
        red                     = RGBGetRValue(colors[0]);
        green                   = RGBGetGValue(colors[0]);
        blue                    = RGBGetBValue(colors[0]);
    }

    if(modes[active_mode].flags & MODE_FLAG_HAS_BRIGHTNESS)
    {
        red                     = (unsigned char)((red   * brightness) / AVERMEDIA_GC570D_BRIGHTNESS_MAX);
        green                   = (unsigned char)((green * brightness) / AVERMEDIA_GC570D_BRIGHTNESS_MAX);
        blue                    = (unsigned char)((blue  * brightness) / AVERMEDIA_GC570D_BRIGHTNESS_MAX);
    }

    controller->SetColor(red, green, blue, speed);
    controller->SetMode((unsigned char)modes[active_mode].value);
}

void RGBController_AVerMediaGC570D::DeviceUpdateZoneLEDs(int zone)
{
    (void)zone;

    DeviceUpdateLEDs();
}

void RGBController_AVerMediaGC570D::DeviceUpdateSingleLED(int led)
{
    (void)led;

    DeviceUpdateLEDs();
}

void RGBController_AVerMediaGC570D::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}
