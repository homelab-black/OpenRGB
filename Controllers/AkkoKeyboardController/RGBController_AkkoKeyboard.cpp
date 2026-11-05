/*---------------------------------------------------------*\
| RGBController_AkkoKeyboard.cpp                             |
|                                                           |
|   RGBController for Akko / RongYuan keyboards.            |
|                                                           |
|   Color: OpenRGB stores colors as 0x00BBGGRR; the         |
|   RGBGetRValue/G/B macros extract the channels in the      |
|   right order, so no manual R<->B swap is needed here.     |
|                                                           |
|   Model-specific quirks handled:                          |
|     - main-zone speed byte is inverted (4 = slowest);      |
|       side bars are normal (4 = fastest)                   |
|     - side bars only support modes 0-5; higher modes are   |
|       skipped on the side zone (left untouched)            |
|     - side-bar Ripple (5) only animates with dazzle on,    |
|       so it is forced to dazzle                            |
|                                                           |
|   Roz                                         20 Jun 2026 |
|                                                           |
|   Protocol by the echtzeit-solutions/monsgeek-akko-linux  |
|   project (docs/PROTOCOL.md) and the SonixQMK community.  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "RGBController_AkkoKeyboard.h"

/*-----------------------------------------------------------*\
| Effect table. `value` is the device mode byte; `color` is   |
| whether the effect honors the custom RGB bytes. Modes with  |
| value > 5 do not work on the side bars (skipped there).     |
| User Picture (13) and Per-Key Color (25) are omitted: they  |
| are not writable on stock firmware.                         |
\*-----------------------------------------------------------*/
struct AkkoModeDef
{
    const char*     name;
    int             value;
    bool            color;
};

static const AkkoModeDef akko_modes[] =
{
    { "Off",            0,  false },
    { "Direct",         1,  true  },
    { "Breathing",      2,  true  },
    { "Neon",           3,  false },
    { "Wave",           4,  true  },
    { "Ripple",         5,  true  },
    { "Raindrop",       6,  true  },
    { "Snake",          7,  true  },
    { "Reactive",       8,  true  },
    { "Converge",       9,  true  },
    { "Sine Wave",      10, true  },
    { "Kaleidoscope",   11, true  },
    { "Line Wave",      12, true  },
    { "Laser",          14, true  },
    { "Circle Wave",    15, true  },
    { "Rainbow",        16, false },
    { "Rain Down",      17, true  },
    { "Meteor",         18, true  },
    { "Reactive Off",   19, true  },
    { "Music Patterns", 20, true  },
    { "Screen Sync",    21, false },
    { "Music Bars",     22, true  },
    { "Train",          23, true  },
    { "Fireworks",      24, true  },
};

RGBController_AkkoKeyboard::RGBController_AkkoKeyboard(AkkoKeyboardController* controller_ptr)
{
    controller  = controller_ptr;

    name        = controller->GetNameString();
    vendor      = "Akko";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "Akko / RongYuan open-protocol keyboard";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();
    version     = controller->GetFirmwareVersion();

    SetupModes();
    SetupZones();

    /*-------------------------------------------------------*\
    | Seed each zone's color from the keyboard's current      |
    | custom RGB so the UI opens with the real colors. This   |
    | is best-effort; defaults stay if the read fails or the  |
    | zone is on a palette/dazzle source.                     |
    \*-------------------------------------------------------*/
    for(int zone_idx = 0; zone_idx < AKKO_ZONE_COUNT; zone_idx++)
    {
        AkkoLedParam param;

        if(controller->GetLED(zone_idx, param) && (param.options & 0x0F) == AKKO_OPT_CUSTOM_RGB)
        {
            colors[zone_idx] = ToRGBColor(param.red, param.green, param.blue);
        }
    }
}

RGBController_AkkoKeyboard::~RGBController_AkkoKeyboard()
{
    delete controller;
}

void RGBController_AkkoKeyboard::SetupModes()
{
    int mode_count = (int)(sizeof(akko_modes) / sizeof(akko_modes[0]));

    for(int def_idx = 0; def_idx < mode_count; def_idx++)
    {
        const AkkoModeDef& def = akko_modes[def_idx];

        mode new_mode;
        new_mode.name  = def.name;
        new_mode.value = def.value;
        new_mode.flags = 0;

        if(def.value == 0)
        {
            new_mode.color_mode = MODE_COLORS_NONE;
            modes.push_back(new_mode);
            continue;
        }

        new_mode.flags         |= MODE_FLAG_HAS_BRIGHTNESS;
        new_mode.brightness_min = 0;
        new_mode.brightness_max = AKKO_MAX_BRIGHTNESS;
        new_mode.brightness     = AKKO_MAX_BRIGHTNESS;

        /*---------------------------------------------------*\
        | "Direct" (Constant) is a static color with no       |
        | animation, so it gets no speed parameter.           |
        \*---------------------------------------------------*/
        if(def.value != 1)
        {
            new_mode.flags    |= MODE_FLAG_HAS_SPEED;
            new_mode.speed_min = 0;
            new_mode.speed_max = AKKO_MAX_SPEED;
            new_mode.speed     = 2;
        }

        if(def.color)
        {
            /*-----------------------------------------------*\
            | Per-LED color (the zone color) plus a Random    |
            | option that maps to the keyboard's dazzle       |
            | (rainbow) source.                               |
            \*-----------------------------------------------*/
            new_mode.flags     |= MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
            new_mode.color_mode = MODE_COLORS_PER_LED;
        }
        else
        {
            new_mode.color_mode = MODE_COLORS_NONE;
        }

        modes.push_back(new_mode);
    }

    /*-------------------------------------------------------*\
    | Default to Direct (static color).                       |
    \*-------------------------------------------------------*/
    for(std::size_t mode_idx = 0; mode_idx < modes.size(); mode_idx++)
    {
        if(modes[mode_idx].value == 1)
        {
            active_mode = (int)mode_idx;
            break;
        }
    }
}

void RGBController_AkkoKeyboard::SetupZones()
{
    const char* akko_zone_names[AKKO_ZONE_COUNT] = { "Keyboard", "Side Bars" };

    for(int zone_idx = 0; zone_idx < AKKO_ZONE_COUNT; zone_idx++)
    {
        zone new_zone;
        new_zone.name       = akko_zone_names[zone_idx];
        new_zone.type       = ZONE_TYPE_SINGLE;
        new_zone.leds_min   = 1;
        new_zone.leds_max   = 1;
        new_zone.leds_count = 1;
        new_zone.matrix_map = nullptr;
        zones.push_back(new_zone);

        led new_led;
        new_led.name = akko_zone_names[zone_idx];
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_AkkoKeyboard::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /* Fixed single-LED zones; nothing to resize. */
}

void RGBController_AkkoKeyboard::SendZone(int zone)
{
    if(zone < 0 || zone >= AKKO_ZONE_COUNT)
    {
        return;
    }

    const mode&     current_mode = modes[active_mode];
    unsigned char   device_mode  = (unsigned char)current_mode.value;

    /*-------------------------------------------------------*\
    | Side bars only support modes 0-5; leave them untouched  |
    | for higher modes.                                       |
    \*-------------------------------------------------------*/
    if(zone == AKKO_ZONE_SIDE && current_mode.value > 5)
    {
        return;
    }

    unsigned char brightness = (current_mode.flags & MODE_FLAG_HAS_BRIGHTNESS)
                             ? (unsigned char)current_mode.brightness
                             : AKKO_MAX_BRIGHTNESS;

    /*-------------------------------------------------------*\
    | UI speed: 4 = fastest. Main zone's byte is inverted;    |
    | the side bars use it directly.                          |
    \*-------------------------------------------------------*/
    unsigned char ui_speed   = (current_mode.flags & MODE_FLAG_HAS_SPEED) ? (unsigned char)current_mode.speed : 0;
    unsigned char speed_byte = (zone == AKKO_ZONE_MAIN)
                             ? (unsigned char)(AKKO_MAX_SPEED - ui_speed)
                             : ui_speed;

    bool force_dazzle = (zone == AKKO_ZONE_SIDE && current_mode.value == 5); /* side Ripple */

    unsigned char options;
    unsigned char red   = 0;
    unsigned char green = 0;
    unsigned char blue  = 0;

    if(current_mode.color_mode == MODE_COLORS_RANDOM || force_dazzle)
    {
        options = AKKO_OPT_DAZZLE;
    }
    else
    {
        options = AKKO_OPT_CUSTOM_RGB;

        RGBColor color = colors[zone]; /* one LED per zone -> led index == zone */
        red   = RGBGetRValue(color);
        green = RGBGetGValue(color);
        blue  = RGBGetBValue(color);
    }

    controller->SetLED(zone, device_mode, speed_byte, brightness, options, red, green, blue);
}

void RGBController_AkkoKeyboard::DeviceUpdateLEDs()
{
    for(int zone_idx = 0; zone_idx < AKKO_ZONE_COUNT; zone_idx++)
    {
        SendZone(zone_idx);
    }
}

void RGBController_AkkoKeyboard::UpdateZoneLEDs(int zone)
{
    SendZone(zone);
}

void RGBController_AkkoKeyboard::UpdateSingleLED(int led)
{
    SendZone(led); /* one LED per zone -> led index == zone */
}

void RGBController_AkkoKeyboard::DeviceUpdateMode()
{
    DeviceUpdateLEDs();
}
