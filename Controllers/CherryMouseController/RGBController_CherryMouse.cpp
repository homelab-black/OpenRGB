/*---------------------------------------------------------*\
| RGBController_CherryMouse.cpp                             |
|                                                           |
|   RGBController for Cherry mouse                          |
|                                                           |
|   Rik Vanden Boer                             03 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_CherryMouse.h"

/**------------------------------------------------------------------*\
    @name Cherry Mouse
    @category Mouse
    @type USB
    @save :robot:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectCherryMice
    @comment The mouse stores the active effect in its own memory.
\*-------------------------------------------------------------------*/

RGBController_CherryMouse::RGBController_CherryMouse(CherryMouseController* controller_ptr)
{
    controller  = controller_ptr;

    name        = controller->GetDeviceName();
    vendor      = "Cherry";
    type        = DEVICE_TYPE_MOUSE;
    description = "Cherry Mouse Device";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();

    mode Static;
    Static.name             = "Static";
    Static.value            = CHERRY_MOUSE_MODE_STATIC;
    Static.flags            = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Static.brightness_min   = CHERRY_MOUSE_BRIGHTNESS_LOWEST;
    Static.brightness_max   = CHERRY_MOUSE_BRIGHTNESS_HIGHEST;
    Static.brightness       = CHERRY_MOUSE_BRIGHTNESS_HIGHEST;
    Static.colors_min       = 1;
    Static.colors_max       = 1;
    Static.color_mode       = MODE_COLORS_MODE_SPECIFIC;
    Static.colors.resize(1);
    modes.push_back(Static);

    mode Breathing;
    Breathing.name           = "Breathing";
    Breathing.value          = CHERRY_MOUSE_MODE_BREATHING;
    Breathing.flags          = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Breathing.speed_min      = CHERRY_MOUSE_SPEED_SLOWEST;
    Breathing.speed_max      = CHERRY_MOUSE_SPEED_FASTEST;
    Breathing.speed          = CHERRY_MOUSE_SPEED_NORMAL;
    Breathing.brightness_min = CHERRY_MOUSE_BRIGHTNESS_LOWEST;
    Breathing.brightness_max = CHERRY_MOUSE_BRIGHTNESS_HIGHEST;
    Breathing.brightness     = CHERRY_MOUSE_BRIGHTNESS_HIGHEST;
    Breathing.colors_min     = 1;
    Breathing.colors_max     = 1;
    Breathing.color_mode     = MODE_COLORS_MODE_SPECIFIC;
    Breathing.colors.resize(1);
    modes.push_back(Breathing);

    mode Spectrum;
    Spectrum.name            = "Spectrum Cycle";
    Spectrum.value           = CHERRY_MOUSE_MODE_SPECTRUM;
    Spectrum.flags           = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Spectrum.speed_min       = CHERRY_MOUSE_SPEED_SLOWEST;
    Spectrum.speed_max       = CHERRY_MOUSE_SPEED_FASTEST;
    Spectrum.speed           = CHERRY_MOUSE_SPEED_NORMAL;
    Spectrum.brightness_min  = CHERRY_MOUSE_BRIGHTNESS_LOWEST;
    Spectrum.brightness_max  = CHERRY_MOUSE_BRIGHTNESS_HIGHEST;
    Spectrum.brightness      = CHERRY_MOUSE_BRIGHTNESS_HIGHEST;
    Spectrum.color_mode      = MODE_COLORS_NONE;
    modes.push_back(Spectrum);

    SetupZones();
}

RGBController_CherryMouse::~RGBController_CherryMouse()
{
    Shutdown();

    delete controller;
}

void RGBController_CherryMouse::SetupZones()
{
    zone new_zone;

    new_zone.name       = "Mouse";
    new_zone.type       = ZONE_TYPE_SINGLE;
    new_zone.leds_min   = 1;
    new_zone.leds_max   = 1;
    new_zone.leds_count = 1;

    zones.push_back(new_zone);

    led new_led;
    new_led.name        = "Logo";
    leds.push_back(new_led);

    SetupColors();
}

void RGBController_CherryMouse::DeviceUpdateLEDs()
{
    /*---------------------------------------------------------*\
    | No direct mode: per-LED colours are applied through the   |
    | active effect's colour.                                   |
    \*---------------------------------------------------------*/
    DeviceUpdateMode();
}

void RGBController_CherryMouse::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateMode();
}

void RGBController_CherryMouse::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateMode();
}

void RGBController_CherryMouse::DeviceUpdateMode()
{
    unsigned char red = 0x00;
    unsigned char grn = 0x00;
    unsigned char blu = 0x00;

    if(modes[active_mode].colors.size() > 0)
    {
        red = RGBGetRValue(modes[active_mode].colors[0]);
        grn = RGBGetGValue(modes[active_mode].colors[0]);
        blu = RGBGetBValue(modes[active_mode].colors[0]);
    }

    /*---------------------------------------------------------*\
    | Direction byte is always 0x01 in captures from the CHERRY |
    | Utility; random-colour flag 0x00.                         |
    \*---------------------------------------------------------*/
    controller->SetMode
                    (
                    modes[active_mode].value,
                    modes[active_mode].brightness,
                    modes[active_mode].speed,
                    0x01,
                    0x00,
                    red,
                    grn,
                    blu
                    );
}
