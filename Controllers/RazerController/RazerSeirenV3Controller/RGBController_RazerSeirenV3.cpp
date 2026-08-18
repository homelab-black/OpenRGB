/*---------------------------------------------------------*\
| RGBController_RazerSeirenV3.cpp                           |
|                                                           |
|   RGBController for Razer Seiren V3 Chroma                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_RazerSeirenV3.h"
#include "RazerDevices.h"

RGBController_RazerSeirenV3::RGBController_RazerSeirenV3(RazerSeirenV3Controller* controller_ptr)
{
    controller              = controller_ptr;

    name                    = controller->GetName();
    vendor                  = "Razer";
    type                    = controller->GetDeviceType();
    description             = "Razer Device";
    location                = controller->GetDeviceLocation();
    version                 = controller->GetFirmwareString();
    serial                  = controller->GetSerialString();
    uint8_t max_brightness  = controller->GetMaxBrightness();

    mode Direct;
    Direct.name             = "Direct";
    Direct.value            = RAZER_SEIREN_V3_MODE_DIRECT;
    Direct.flags            = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Direct.color_mode       = MODE_COLORS_PER_LED;
    Direct.brightness_min   = 0;
    Direct.brightness_max   = max_brightness;
    Direct.brightness       = max_brightness;
    modes.push_back(Direct);

    mode Off;
    Off.name                = "Off";
    Off.value               = RAZER_SEIREN_V3_MODE_OFF;
    Off.flags               = 0;
    Off.color_mode          = MODE_COLORS_NONE;
    modes.push_back(Off);

    mode Static;
    Static.name             = "Static";
    Static.value            = RAZER_SEIREN_V3_MODE_STATIC;
    Static.flags            = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Static.color_mode       = MODE_COLORS_MODE_SPECIFIC;
    Static.colors_min       = 1;
    Static.colors_max       = 1;
    Static.colors.resize(1);
    Static.brightness_min   = 0;
    Static.brightness_max   = max_brightness;
    Static.brightness       = max_brightness;
    modes.push_back(Static);

    mode Breathing;
    Breathing.name           = "Breathing";
    Breathing.value          = RAZER_SEIREN_V3_MODE_BREATHING;
    Breathing.flags          = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Breathing.color_mode     = MODE_COLORS_MODE_SPECIFIC;
    Breathing.colors_min     = 1;
    Breathing.colors_max     = 2;
    Breathing.colors.resize(2);
    Breathing.speed_min      = 0;
    Breathing.speed_max      = 0;
    Breathing.speed           = 0;
    Breathing.brightness_min = 0;
    Breathing.brightness_max = max_brightness;
    Breathing.brightness     = max_brightness;
    modes.push_back(Breathing);

    mode SpectrumCycle;
    SpectrumCycle.name           = "Spectrum Cycle";
    SpectrumCycle.value          = RAZER_SEIREN_V3_MODE_SPECTRUM_CYCLE;
    SpectrumCycle.flags          = MODE_FLAG_HAS_BRIGHTNESS;
    SpectrumCycle.color_mode     = MODE_COLORS_NONE;
    SpectrumCycle.brightness_min = 0;
    SpectrumCycle.brightness_max = max_brightness;
    SpectrumCycle.brightness     = max_brightness;
    modes.push_back(SpectrumCycle);

    mode AudioReactive;
    AudioReactive.name           = "Audio Reactive";
    AudioReactive.value          = RAZER_SEIREN_V3_MODE_AUDIO_REACTIVE;
    AudioReactive.flags          = MODE_FLAG_HAS_BRIGHTNESS;
    AudioReactive.color_mode     = MODE_COLORS_NONE;
    AudioReactive.brightness_min = 0;
    AudioReactive.brightness_max = max_brightness;
    AudioReactive.brightness     = max_brightness;
    modes.push_back(AudioReactive);

    SetupZones();
}

RGBController_RazerSeirenV3::~RGBController_RazerSeirenV3()
{
    Shutdown();

    delete controller;
}

void RGBController_RazerSeirenV3::SetupZones()
{
    unsigned int device_index = controller->GetDeviceIndex();

    /*-----------------------------------------------------*\
    | The Seiren V3 Chroma has TWO rings:                   |
    |   zone 0: bottom ring (LEDs 0-7)                      |
    |   zone 1: top ring (LEDs 8-15)                        |
    | Both are linear zones of 8 LEDs.                      |
    \*-----------------------------------------------------*/
    for(unsigned int zone_id = 0; zone_id < RAZER_MAX_ZONES; zone_id++)
    {
        if(device_list[device_index]->zones[zone_id] != NULL)
        {
            zone new_zone;

            new_zone.name       = device_list[device_index]->zones[zone_id]->name;
            new_zone.type       = device_list[device_index]->zones[zone_id]->type;

            new_zone.leds_count = device_list[device_index]->zones[zone_id]->cols;
            new_zone.leds_min   = new_zone.leds_count;
            new_zone.leds_max   = new_zone.leds_count;

            zones.push_back(new_zone);

            for(unsigned int col_id = 0; col_id < device_list[device_index]->zones[zone_id]->cols; col_id++)
            {
                led* new_led = new led();

                new_led->name = device_list[device_index]->zones[zone_id]->name;

                if(zones[zone_id].leds_count > 1)
                {
                    new_led->name.append(" LED ");
                    new_led->name.append(std::to_string(col_id + 1));
                }

                leds.push_back(*new_led);
            }
        }
    }

    SetupColors();
}

void RGBController_RazerSeirenV3::DeviceUpdateLEDs()
{
    if(modes[active_mode].value == RAZER_SEIREN_V3_MODE_DIRECT)
    {
        /*-----------------------------------------------------*\
        | The Seiren has no usable hardware brightness command. |
        | Implement brightness in software by scaling RGB values |
        | before pushing frames.                                |
        \*-----------------------------------------------------*/
        unsigned char brightness = modes[active_mode].brightness;
        if(modes[active_mode].brightness_max == 0)
        {
            brightness = 100;
        }

        std::vector<RGBColor> scaled_colors;
        for(unsigned int led_idx = 0; led_idx < colors.size(); led_idx++)
        {
            unsigned char r = (RGBGetRValue(colors[led_idx]) * brightness) / modes[active_mode].brightness_max;
            unsigned char g = (RGBGetGValue(colors[led_idx]) * brightness) / modes[active_mode].brightness_max;
            unsigned char b = (RGBGetBValue(colors[led_idx]) * brightness) / modes[active_mode].brightness_max;

            scaled_colors.push_back(ToRGBColor(r, g, b));
        }

        controller->SetDirect(scaled_colors.data());
        controller->UpdateKeepaliveColors(scaled_colors.data());
    }
}

void RGBController_RazerSeirenV3::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RazerSeirenV3::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RazerSeirenV3::DeviceUpdateMode()
{
    /*-----------------------------------------------------*\
    | Keepalive only applies to Direct mode. Any other      |
    | mode switch must stop the frame pusher or it will     |
    | fight the new effect.                                 |
    \*-----------------------------------------------------*/
    if(modes[active_mode].value != RAZER_SEIREN_V3_MODE_DIRECT)
    {
        controller->StopKeepalive();
    }

    switch(modes[active_mode].value)
    {
        case RAZER_SEIREN_V3_MODE_DIRECT:
            controller->SetModeCustom();
            break;

        case RAZER_SEIREN_V3_MODE_OFF:
            controller->SetModeOff();
            break;

        case RAZER_SEIREN_V3_MODE_STATIC:
            if(modes[active_mode].colors.size() == 1)
            {
                unsigned char red = RGBGetRValue(modes[active_mode].colors[0]);
                unsigned char grn = RGBGetGValue(modes[active_mode].colors[0]);
                unsigned char blu = RGBGetBValue(modes[active_mode].colors[0]);

                controller->SetModeStatic(red, grn, blu);
            }
            break;

        case RAZER_SEIREN_V3_MODE_BREATHING:
            if(modes[active_mode].colors.size() == 2)
            {
                unsigned char r1 = RGBGetRValue(modes[active_mode].colors[0]);
                unsigned char g1 = RGBGetGValue(modes[active_mode].colors[0]);
                unsigned char b1 = RGBGetBValue(modes[active_mode].colors[0]);
                unsigned char r2 = RGBGetRValue(modes[active_mode].colors[1]);
                unsigned char g2 = RGBGetGValue(modes[active_mode].colors[1]);
                unsigned char b2 = RGBGetBValue(modes[active_mode].colors[1]);

                controller->SetModeBreathingTwoColors(r1, g1, b1, r2, g2, b2, modes[active_mode].speed);
            }
            else if(modes[active_mode].colors.size() == 1)
            {
                unsigned char red = RGBGetRValue(modes[active_mode].colors[0]);
                unsigned char grn = RGBGetGValue(modes[active_mode].colors[0]);
                unsigned char blu = RGBGetBValue(modes[active_mode].colors[0]);

                controller->SetModeBreathingOneColor(red, grn, blu, modes[active_mode].speed);
            }
            else if(colors.size() > 0)
            {
                /*-----------------------------------------------------*\
                | No mode-specific colors set (user only changed the    |
                | mode, not the swatch) — fall back to the first LED    |
                | color so breathing actually does something.           |
                \*-----------------------------------------------------*/
                unsigned char red = RGBGetRValue(colors[0]);
                unsigned char grn = RGBGetGValue(colors[0]);
                unsigned char blu = RGBGetBValue(colors[0]);

                controller->SetModeBreathingOneColor(red, grn, blu, modes[active_mode].speed);
            }
            break;

        case RAZER_SEIREN_V3_MODE_SPECTRUM_CYCLE:
            controller->SetModeSpectrumCycle();
            break;

        case RAZER_SEIREN_V3_MODE_AUDIO_REACTIVE:
            controller->SetModeAudioReactive();
            break;
    }

    /*-----------------------------------------------------*\
    | Apply brightness after every mode change (hardware    |
    | brightness command, class 0x0F id 0x04)               |
    \*-----------------------------------------------------*/
    if(modes[active_mode].flags & MODE_FLAG_HAS_BRIGHTNESS)
    {
        controller->SetBrightness(modes[active_mode].brightness);
    }
}
