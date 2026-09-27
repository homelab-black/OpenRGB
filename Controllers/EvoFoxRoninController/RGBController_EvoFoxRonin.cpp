/*---------------------------------------------------------*\
| RGBController_EvoFoxRonin.cpp                             |
|                                                           |
|   RGBController for EvoFox Ronin TKL RGB keyboard         |
|                                                           |
|   <Atif Usmani> (<AtifUsmani>)               <30 09 2026> |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_EvoFoxRonin.h"

/**------------------------------------------------------------------*\
    @name EvoFox Ronin TKL
    @category Keyboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectEvoFoxRoninControllers
    @comment
\*-------------------------------------------------------------------*/

RGBController_EvoFoxRonin::RGBController_EvoFoxRonin(EvoFoxRoninController* controller_ptr)
{
    controller   = controller_ptr;
    name         = controller->GetDeviceName();
    vendor       = "EvoFox";
    type         = DEVICE_TYPE_KEYBOARD;
    description  = "EvoFox Ronin TKL Keyboard";
    location     = controller->GetDeviceLocation();
    serial       = controller->GetSerialString();

    // supports_direction was determined empirically on real hardware:
    // only Corrugated and Cloud visibly reverse when the direction byte
    // is toggled; the other effects showed no visible change.
    const struct { const char* name; unsigned int effect; bool supports_direction; } effects[] =
    {
        { "Corrugated", EVOFOX_RONIN_EFFECT_CORRUGATED, true  },
        { "Cloud",      EVOFOX_RONIN_EFFECT_CLOUD,       true  },
        { "Serpentine", EVOFOX_RONIN_EFFECT_SERPENTINE,  false },
        { "Spectrum",   EVOFOX_RONIN_EFFECT_SPECTRUM,    false },
        { "Breath",     EVOFOX_RONIN_EFFECT_BREATH,      false },
        { "Static",     EVOFOX_RONIN_EFFECT_STATIC,      false },
        { "Stars",      EVOFOX_RONIN_EFFECT_STARS,       false },
        { "Flowers",    EVOFOX_RONIN_EFFECT_FLOWERS,     false },
        { "Roll",       EVOFOX_RONIN_EFFECT_ROLL,        false },
        { "Wave",       EVOFOX_RONIN_EFFECT_WAVE,        false },
        { "Wave (Alt)", EVOFOX_RONIN_EFFECT_WAVE_ALT,    false },
        { "Rain",       EVOFOX_RONIN_EFFECT_RAIN,        false },
        { "Scan",       EVOFOX_RONIN_EFFECT_SCAN,        false },
    };

    for(const auto &effect : effects)
    {
        mode m;
        m.name          = effect.name;
        m.value         = effect.effect;
        m.flags         = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
        if(effect.supports_direction)
        {
            m.flags |= MODE_FLAG_HAS_DIRECTION_LR;
        }
        m.brightness_min = 0;
        m.brightness_max = 4;
        m.brightness     = 4;
        m.speed_min      = 4;
        m.speed_max      = 0;
        m.speed          = 2;
        m.direction      = MODE_DIRECTION_LEFT;
        m.color_mode     = MODE_COLORS_MODE_SPECIFIC;
        m.colors_min     = 1;
        m.colors_max     = 1;
        m.colors.resize(1);
        m.colors[0]      = ToRGBColor(255, 255, 255);
        modes.push_back(m);
    }

    SetupZones();
}

RGBController_EvoFoxRonin::~RGBController_EvoFoxRonin()
{
    Shutdown();

    delete controller;
}

void RGBController_EvoFoxRonin::SetupZones()
{
    zone keyboard;
    keyboard.name        = "Keyboard";
    keyboard.type        = ZONE_TYPE_SINGLE;
    keyboard.leds_min    = 1;
    keyboard.leds_max    = 1;
    keyboard.leds_count  = 1;
    zones.push_back(keyboard);

    led keyboard_led;
    keyboard_led.name = "Keyboard";
    leds.push_back(keyboard_led);
    SetupColors();
}

void RGBController_EvoFoxRonin::DeviceUpdateLEDs()
{
    if(colors.empty())
    {
        return;
    }

    controller->SetLightingConfig(modes[active_mode], colors[0]);
}

void RGBController_EvoFoxRonin::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_EvoFoxRonin::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_EvoFoxRonin::DeviceUpdateMode()
{
    RGBColor color = 0x00FFFFFF;
    if(!modes[active_mode].colors.empty())
    {
        color = modes[active_mode].colors[0];
    }

    controller->SetLightingConfig(modes[active_mode], color);
}
