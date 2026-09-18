/*---------------------------------------------------------*\
| RGBController_HamaUrageReaper330.cpp                      |
|                                                           |
|   RGBController for Hama uRage Reaper 330 mouse           |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_HamaUrageReaper330.h"

/**------------------------------------------------------------------*\
    @name Hama uRage Reaper 330
    @category Mouse
    @type USB
    @save :x:
    @direct :x:
    @effects :warning:
    @detectors DetectHamaUrageReaper330Controllers
    @comment Static has colour and brightness, effects have speed.
        Effect colours are not settable.
\*-------------------------------------------------------------------*/

RGBController_HamaUrageReaper330::RGBController_HamaUrageReaper330(HamaUrageReaper330Controller* controller_ptr)
{
    controller              = controller_ptr;

    name                    = controller->GetNameString();
    vendor                  = "Hama";
    type                    = DEVICE_TYPE_MOUSE;
    description             = "Hama uRage Reaper 330 Gaming Mouse";
    location                = controller->GetDeviceLocation();
    serial                  = controller->GetSerialString();

    /*-----------------------------------------------------*\
    | Static has a colour and 5 brightness steps.  The      |
    | effects have 4 speed steps.  On the device 0 is the   |
    | fastest and 3 the slowest, so the range is reversed   |
    | (speed_min is the slow end).                          |
    \*-----------------------------------------------------*/
    mode Off;
    Off.name                = "Off";
    Off.value               = HAMA_URAGE_REAPER_330_MODE_OFF;
    Off.flags               = 0;
    Off.color_mode          = MODE_COLORS_NONE;
    modes.push_back(Off);

    mode Static;
    Static.name             = "Static";
    Static.value            = HAMA_URAGE_REAPER_330_MODE_STATIC;
    Static.flags            = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Static.brightness_min   = HAMA_URAGE_REAPER_330_BRIGHTNESS_MIN;
    Static.brightness_max   = HAMA_URAGE_REAPER_330_BRIGHTNESS_MAX;
    Static.brightness       = HAMA_URAGE_REAPER_330_BRIGHTNESS_MAX;
    Static.color_mode       = MODE_COLORS_PER_LED;
    modes.push_back(Static);

    mode Breathing;
    Breathing.name          = "Breathing";
    Breathing.value         = HAMA_URAGE_REAPER_330_MODE_BREATHING;
    Breathing.flags         = MODE_FLAG_HAS_SPEED;
    Breathing.speed_min     = HAMA_URAGE_REAPER_330_SPEED_SLOWEST;
    Breathing.speed_max     = HAMA_URAGE_REAPER_330_SPEED_FASTEST;
    Breathing.speed         = 1;
    Breathing.color_mode    = MODE_COLORS_NONE;
    modes.push_back(Breathing);

    mode Gliding;
    Gliding.name            = "Gliding";
    Gliding.value           = HAMA_URAGE_REAPER_330_MODE_GLIDING;
    Gliding.flags           = MODE_FLAG_HAS_SPEED;
    Gliding.speed_min       = HAMA_URAGE_REAPER_330_SPEED_SLOWEST;
    Gliding.speed_max       = HAMA_URAGE_REAPER_330_SPEED_FASTEST;
    Gliding.speed           = 1;
    Gliding.color_mode      = MODE_COLORS_NONE;
    modes.push_back(Gliding);

    mode Raindrop;
    Raindrop.name           = "Raindrop";
    Raindrop.value          = HAMA_URAGE_REAPER_330_MODE_RAINDROP;
    Raindrop.flags          = MODE_FLAG_HAS_SPEED;
    Raindrop.speed_min      = HAMA_URAGE_REAPER_330_SPEED_SLOWEST;
    Raindrop.speed_max      = HAMA_URAGE_REAPER_330_SPEED_FASTEST;
    Raindrop.speed          = 1;
    Raindrop.color_mode     = MODE_COLORS_NONE;
    modes.push_back(Raindrop);

    mode KeyReaction;
    KeyReaction.name        = "Key Reaction";
    KeyReaction.value       = HAMA_URAGE_REAPER_330_MODE_KEY_REACTION;
    KeyReaction.flags       = MODE_FLAG_HAS_SPEED;
    KeyReaction.speed_min   = HAMA_URAGE_REAPER_330_SPEED_SLOWEST;
    KeyReaction.speed_max   = HAMA_URAGE_REAPER_330_SPEED_FASTEST;
    KeyReaction.speed       = 1;
    KeyReaction.color_mode  = MODE_COLORS_NONE;
    modes.push_back(KeyReaction);

    mode Rolling;
    Rolling.name            = "Rolling Back and Forth";
    Rolling.value           = HAMA_URAGE_REAPER_330_MODE_ROLLING;
    Rolling.flags           = MODE_FLAG_HAS_SPEED;
    Rolling.speed_min       = HAMA_URAGE_REAPER_330_SPEED_SLOWEST;
    Rolling.speed_max       = HAMA_URAGE_REAPER_330_SPEED_FASTEST;
    Rolling.speed           = 1;
    Rolling.color_mode      = MODE_COLORS_NONE;
    modes.push_back(Rolling);

    SetupZones();
}

RGBController_HamaUrageReaper330::~RGBController_HamaUrageReaper330()
{
    Shutdown();

    delete controller;
}

void RGBController_HamaUrageReaper330::SetupZones()
{
    zone mouse_zone;
    mouse_zone.name         = "Mouse";
    mouse_zone.type         = ZONE_TYPE_SINGLE;
    mouse_zone.leds_min     = 1;
    mouse_zone.leds_max     = 1;
    mouse_zone.leds_count   = 1;
    zones.push_back(mouse_zone);

    led mouse_led;
    mouse_led.name          = "Mouse";
    leds.push_back(mouse_led);

    SetupColors();

    /*-----------------------------------------------------*\
    | Start with white rather than black, so selecting      |
    | Static does not turn the lighting off                 |
    \*-----------------------------------------------------*/
    colors[0] = ToRGBColor(255, 255, 255);
}

void RGBController_HamaUrageReaper330::DeviceUpdateLEDs()
{
    /*-----------------------------------------------------*\
    | Colour is only user-settable in Static mode, and on   |
    | this device a colour change is the same command as a  |
    | mode change, so resend the mode with the new colour   |
    \*-----------------------------------------------------*/
    if(modes[active_mode].value == HAMA_URAGE_REAPER_330_MODE_STATIC)
    {
        DeviceUpdateMode();
    }
}

void RGBController_HamaUrageReaper330::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_HamaUrageReaper330::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_HamaUrageReaper330::DeviceUpdateMode()
{
    unsigned char mode_value    = (unsigned char)modes[active_mode].value;
    unsigned char speed         = 0;
    unsigned char brightness    = HAMA_URAGE_REAPER_330_BRIGHTNESS_MAX;

    /*-----------------------------------------------------*\
    | Speed only applies to the effects, brightness only to |
    | Static.  The vendor software sends speed 0 for Off    |
    | and Static, and maximum brightness for everything     |
    | except Static.                                        |
    \*-----------------------------------------------------*/
    if(modes[active_mode].flags & MODE_FLAG_HAS_SPEED)
    {
        speed                   = (unsigned char)modes[active_mode].speed;
    }

    if(modes[active_mode].flags & MODE_FLAG_HAS_BRIGHTNESS)
    {
        brightness              = (unsigned char)modes[active_mode].brightness;
    }

    controller->SetMode(mode_value,
                        speed,
                        brightness,
                        RGBGetRValue(colors[0]),
                        RGBGetGValue(colors[0]),
                        RGBGetBValue(colors[0]));
}
