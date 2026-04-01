/*---------------------------------------------------------*\
| RGBController_MSIGE76.cpp                                 |
|                                                           |
|   RGBController for MSI GE76 Raider KLC/ALC              |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later             |
\*---------------------------------------------------------*/

#include "RGBController_MSIGE76.h"
#include "MSIGE76HardwarePresets.h"
#include "MSIGE76KeyboardLayout.h"
#include "RGBControllerKeyNames.h"

/**------------------------------------------------------------------*\
    @name MSI GE76 Raider
    @category Laptop
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectMSIGE76KLC,DetectMSIGE76ALC
    @comment SteelSeries KLC (per-key) and ALC (29-zone Aurora bar). Direct
             uses the GE76 matrix / bar slot map. Hardware modes replay
             msi-perkeyrgb captures (not generated from the map); firmware
             stacks 0x0b effect layers, so non-default presets clear static
             black, apply vendor Default, then the chosen effect. Same presets
             are sent on ALC (same protocol family). Direct bar: 29 LEDs,
             slots 0–28, HID KC 1–29. ALC presets patch numpad packet slots
             (constant in MSIGE76Controller.cpp). Source: msi-perkeyrgb
             presets/1.json.
\*-------------------------------------------------------------------*/

RGBController_MSIGE76::RGBController_MSIGE76(MSIGE76Controller* controller_ptr)
    : controller(controller_ptr)
{
    name        = controller->GetDeviceName();
    vendor      = "MSI";
    type        = DEVICE_TYPE_LAPTOP;
    description = "MSI GE76 Raider SteelSeries RGB Device";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();

    mode Direct;
    Direct.name             = "Direct";
    Direct.value            = 0;
    Direct.flags            = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode       = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    if(controller->GetLEDCount() == MSIGE76_KLC_LED_COUNT
       || controller->GetLEDCount() == MSIGE76_ALC_LED_COUNT)
    {
        for(unsigned int pi = 0; pi < MSIGE76_HARDWARE_PRESET_COUNT; pi++)
        {
            mode m;
            m.name        = MSIGE76_HARDWARE_PRESETS[pi].name;
            m.value       = (int)(pi + 1);
            m.flags       = 0;
            m.color_mode  = MODE_COLORS_NONE;
            modes.push_back(m);
        }
    }

    SetupZones();
}

RGBController_MSIGE76::~RGBController_MSIGE76()
{
    delete controller;
}

void RGBController_MSIGE76::SetupZones()
{
    unsigned int count = controller->GetLEDCount();

    if(count == MSIGE76_KLC_LED_COUNT)
    {
        zone keyboard_zone;
        keyboard_zone.name        = ZONE_EN_KEYBOARD;
        keyboard_zone.type        = ZONE_TYPE_MATRIX;

        matrix_map_type* mm       = new matrix_map_type;
        keyboard_zone.matrix_map  = mm;
        mm->height                = MSIGE76_KLC_MATRIX_ROWS;
        mm->width                 = MSIGE76_KLC_MATRIX_COLS;
        mm->map                   = new unsigned int[MSIGE76_KLC_MATRIX_ROWS * MSIGE76_KLC_MATRIX_COLS];

        for(unsigned int i = 0; i < MSIGE76_KLC_MATRIX_ROWS * MSIGE76_KLC_MATRIX_COLS; i++)
        {
            mm->map[i] = MSIGE76_KLC_MATRIX_MAP[i];
        }

        keyboard_zone.leds_min    = MSIGE76_KLC_LED_COUNT;
        keyboard_zone.leds_max    = MSIGE76_KLC_LED_COUNT;
        keyboard_zone.leds_count  = MSIGE76_KLC_LED_COUNT;

        zones.push_back(keyboard_zone);

        for(unsigned int i = 0; i < MSIGE76_KLC_LED_COUNT; i++)
        {
            led zone_led;
            zone_led.name  = MSIGE76_KLC_LED_NAMES[i];
            zone_led.value = i;
            leds.push_back(zone_led);
        }
    }
    else if(count == MSIGE76_ALC_LED_COUNT)
    {
        zone bar_zone;
        bar_zone.name             = name;
        bar_zone.type             = ZONE_TYPE_MATRIX;

        matrix_map_type* mm       = new matrix_map_type;
        bar_zone.matrix_map       = mm;
        mm->height                = MSIGE76_ALC_MATRIX_ROWS;
        mm->width                 = MSIGE76_ALC_MATRIX_COLS;
        mm->map                   = new unsigned int[MSIGE76_ALC_MATRIX_ROWS * MSIGE76_ALC_MATRIX_COLS];

        for(unsigned int i = 0; i < MSIGE76_ALC_MATRIX_ROWS * MSIGE76_ALC_MATRIX_COLS; i++)
        {
            mm->map[i] = i;
        }

        bar_zone.leds_min         = MSIGE76_ALC_LED_COUNT;
        bar_zone.leds_max         = MSIGE76_ALC_LED_COUNT;
        bar_zone.leds_count       = MSIGE76_ALC_LED_COUNT;
        zones.push_back(bar_zone);

        for(unsigned int i = 0; i < MSIGE76_ALC_LED_COUNT; i++)
        {
            led zone_led;
            zone_led.name = std::string("Aurora bar ") + std::to_string(i + 1);
            leds.push_back(zone_led);
        }
    }
    else
    {
        zone device_zone;
        device_zone.name        = name;
        device_zone.type        = ZONE_TYPE_LINEAR;

        device_zone.leds_min     = count;
        device_zone.leds_max     = count;
        device_zone.leds_count   = count;
        device_zone.matrix_map   = NULL;
        zones.push_back(device_zone);

        for(unsigned int i = 0; i < count; i++)
        {
            led zone_led;
            zone_led.name = name + " LED " + std::to_string(i + 1);
            leds.push_back(zone_led);
        }
    }

    SetupColors();
}

void RGBController_MSIGE76::ResizeZone(int /*zone*/, int /*new_size*/)
{
}

void RGBController_MSIGE76::DeviceUpdateLEDs()
{
    if(active_mode < 0 || (unsigned int)active_mode >= modes.size())
    {
        return;
    }
    if(modes[active_mode].value != 0)
    {
        return;
    }
    controller->SetColors(colors, MSIGE76_BRIGHTNESS_MAX);
}

void RGBController_MSIGE76::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_MSIGE76::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_MSIGE76::DeviceUpdateMode()
{
    if(active_mode < 0 || (unsigned int)active_mode >= modes.size())
    {
        return;
    }
    int v = modes[active_mode].value;
    if(v == 0)
    {
        DeviceUpdateLEDs();
        return;
    }
    unsigned int idx = (unsigned int)v - 1;
    if(idx < MSIGE76_HARDWARE_PRESET_COUNT)
    {
        controller->ApplyHardwarePreset(idx);
        SignalUpdate();
    }
}
