/*---------------------------------------------------------*\
| RGBController_Maono.cpp                                   |
|                                                           |
|   RGBController for Maono microphones                     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

/**------------------------------------------------------------------*\
    @name Maono Microphones
    @category Microphone
    @type USB
    @save :x:
    @direct :warning:
    @effects :white_check_mark:
    @detectors DetectMaonoControllers
    @comment Supported: PD200W (tested), PD200W/PD100W/DGM20W
    receivers, PD100W, DGM20W, Wave T1 Mini Pro, DM40, DM40 Pro,
    DM40 Live, DM40 Live Gen2, PD200X, PD100X (all variants),
    DGM20, DGM20G. Devices with custom colour slots (PD200W,
    PD100W, DGM20W, T1 Mini Pro, DM40 family) show any colour in
    Direct mode; the others snap to the nearest built-in preset.
    Only the PD200W has been confirmed on hardware.
\*-------------------------------------------------------------------*/

#include <algorithm>
#include <cmath>
#include "RGBController_Maono.h"

RGBController_Maono::RGBController_Maono(MaonoController* controller_ptr)
{
    controller                  = controller_ptr;
    info                        = controller->GetInfo();

    name                        = controller->GetNameString();
    vendor                      = "Maono";
    type                        = DEVICE_TYPE_MICROPHONE;
    description                 = std::string("Maono Microphone (") + info->plugin + ")";
    location                    = controller->GetDeviceLocation();
    serial                      = controller->GetSerialString();

    bool has_custom             = (info->protocol == MAONO_LIGHT_REGISTER_BLOCK) && (info->custom_slots > 0);
    bool has_brightness         = (info->reg_brightness != MAONO_REG_NONE);

    unsigned int bright_flag    = has_brightness ? MODE_FLAG_HAS_BRIGHTNESS : 0;

    mode Direct;
    Direct.name                 = has_custom ? "Direct" : "Static";
    Direct.value                = MAONO_MODE_DIRECT;
    Direct.flags                = MODE_FLAG_HAS_PER_LED_COLOR | bright_flag;
    Direct.color_mode           = MODE_COLORS_PER_LED;
    Direct.brightness_min       = info->brightness_min;
    Direct.brightness_max       = info->brightness_max;
    Direct.brightness           = info->brightness_max;
    modes.push_back(Direct);

    bool has_breathing          = (info->protocol == MAONO_LIGHT_REGISTER_BLOCK) ? (info->effect_breathing != MAONO_VALUE_NONE)
                                                                                 : (info->num_pulse_colors > 0);

    if(has_breathing)
    {
        mode Breathing;
        Breathing.name          = "Breathing";
        Breathing.value         = MAONO_MODE_BREATHING;
        Breathing.flags         = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | bright_flag;
        Breathing.color_mode    = MODE_COLORS_MODE_SPECIFIC;
        Breathing.colors_min    = 1;
        Breathing.colors_max    = 1;
        Breathing.colors.resize(1);
        Breathing.colors[0]     = ToRGBColor(0x00, 0xFF, 0x00);
        Breathing.brightness_min= info->brightness_min;
        Breathing.brightness_max= info->brightness_max;
        Breathing.brightness    = info->brightness_max;
        modes.push_back(Breathing);
    }

    bool has_cycle              = (info->protocol == MAONO_LIGHT_REGISTER_BLOCK) ? (info->effect_cycle != MAONO_VALUE_NONE)
                                                                                 : (info->cycle_color_value != MAONO_VALUE_NONE);

    if(has_cycle)
    {
        mode ColorCycle;
        ColorCycle.name         = "Color Cycle";
        ColorCycle.value        = MAONO_MODE_COLOR_CYCLE;
        ColorCycle.flags        = bright_flag;
        ColorCycle.color_mode   = MODE_COLORS_NONE;
        ColorCycle.brightness_min = info->brightness_min;
        ColorCycle.brightness_max = info->brightness_max;
        ColorCycle.brightness   = info->brightness_max;
        modes.push_back(ColorCycle);
    }

    if(info->protocol == MAONO_LIGHT_REGISTER_BLOCK && info->effect_rhythm != MAONO_VALUE_NONE)
    {
        mode Rhythm;
        Rhythm.name             = "Music Rhythm";
        Rhythm.value            = MAONO_MODE_RHYTHM;
        Rhythm.flags            = bright_flag;
        Rhythm.color_mode       = MODE_COLORS_NONE;
        Rhythm.brightness_min   = info->brightness_min;
        Rhythm.brightness_max   = info->brightness_max;
        Rhythm.brightness       = info->brightness_max;
        modes.push_back(Rhythm);
    }

    for(unsigned int special = 0; special < info->num_specials; special++)
    {
        mode Special;
        Special.name            = info->specials[special].name;
        Special.value           = MAONO_MODE_SPECIAL + special;
        Special.flags           = bright_flag;
        Special.color_mode      = MODE_COLORS_NONE;
        Special.brightness_min  = info->brightness_min;
        Special.brightness_max  = info->brightness_max;
        Special.brightness      = info->brightness_max;
        modes.push_back(Special);
    }

    mode Off;
    Off.name                    = "Off";
    Off.value                   = MAONO_MODE_OFF;
    Off.flags                   = 0;
    Off.color_mode              = MODE_COLORS_NONE;
    modes.push_back(Off);

    SetupZones();
}

RGBController_Maono::~RGBController_Maono()
{
    Shutdown();

    delete controller;
}

void RGBController_Maono::SetupZones()
{
    for(unsigned int channel = 0; channel < info->channels; channel++)
    {
        zone ring_zone;

        if(info->channels == 1)
        {
            ring_zone.name      = "Microphone Ring";
        }
        else
        {
            ring_zone.name      = "Transmitter " + std::to_string(channel + 1);
        }

        ring_zone.type          = ZONE_TYPE_SINGLE;
        ring_zone.leds_min      = 1;
        ring_zone.leds_max      = 1;
        ring_zone.leds_count    = 1;
        zones.push_back(ring_zone);

        led ring_led;
        ring_led.name           = ring_zone.name;
        ring_led.value          = channel;
        leds.push_back(ring_led);
    }

    SetupColors();

    for(unsigned int led_idx = 0; led_idx < colors.size(); led_idx++)
    {
        colors[led_idx]         = ToRGBColor(0xFF, 0xFF, 0xFF);
    }
}

/*---------------------------------------------------------*\
| Nearest preset: low-saturation colours pick white (when   |
| the device has it), everything else is matched by hue so  |
| dim colours still land on the right preset.               |
\*---------------------------------------------------------*/
static float MaonoHue(RGBColor color, float* saturation)
{
    int r = RGBGetRValue(color);
    int g = RGBGetGValue(color);
    int b = RGBGetBValue(color);

    int max_c = std::max(r, std::max(g, b));
    int min_c = std::min(r, std::min(g, b));
    int delta = max_c - min_c;

    *saturation = (max_c == 0) ? 0.0f : (float)delta / (float)max_c;

    if(delta == 0)
    {
        return(0.0f);
    }

    float hue;

    if(max_c == r)
    {
        hue = 60.0f * ((float)(g - b) / delta);
    }
    else if(max_c == g)
    {
        hue = 60.0f * (2.0f + (float)(b - r) / delta);
    }
    else
    {
        hue = 60.0f * (4.0f + (float)(r - g) / delta);
    }

    return((hue < 0.0f) ? hue + 360.0f : hue);
}

const MaonoColorPreset* RGBController_Maono::NearestPreset(RGBColor color, const MaonoColorPreset* presets, unsigned int count)
{
    if(count == 0)
    {
        return(nullptr);
    }

    float sat;
    float hue = MaonoHue(color, &sat);

    const MaonoColorPreset* white = nullptr;

    for(unsigned int i = 0; i < count; i++)
    {
        if(presets[i].color == color)
        {
            return(&presets[i]);
        }

        if(presets[i].color == ToRGBColor(0xFF, 0xFF, 0xFF))
        {
            white = &presets[i];
        }
    }

    if(white != nullptr && sat < 0.25f)
    {
        return(white);
    }

    const MaonoColorPreset* best      = &presets[0];
    float                   best_dist = 1000.0f;

    for(unsigned int i = 0; i < count; i++)
    {
        if(&presets[i] == white)
        {
            continue;
        }

        float preset_sat;
        float dist = std::fabs(hue - MaonoHue(presets[i].color, &preset_sat));

        dist = std::min(dist, 360.0f - dist);

        if(dist < best_dist)
        {
            best_dist = dist;
            best      = &presets[i];
        }
    }

    return(best);
}

void RGBController_Maono::ApplyStaticColor(unsigned int channel, RGBColor color)
{
    if(info->protocol == MAONO_LIGHT_REGISTER_BLOCK && info->custom_slots > 0)
    {
        for(unsigned int i = 0; i < info->num_static_colors; i++)
        {
            if(info->static_colors[i].color == color)
            {
                controller->SetColorValue(channel, info->static_colors[i].value);
                return;
            }
        }

        controller->SetCustomColor(channel, RGBGetRValue(color), RGBGetGValue(color), RGBGetBValue(color));
        return;
    }

    const MaonoColorPreset* preset = NearestPreset(color, info->static_colors, info->num_static_colors);

    if(preset != nullptr)
    {
        controller->SetColorValue(channel, preset->value);
    }
}

void RGBController_Maono::ApplyPulseColor(unsigned int channel, RGBColor color)
{
    const MaonoColorPreset* preset = NearestPreset(color, info->pulse_colors, info->num_pulse_colors);

    if(preset != nullptr)
    {
        controller->SetColorValue(channel, preset->value);
    }
}

void RGBController_Maono::DeviceUpdateLEDs()
{
    if(modes[active_mode].value != MAONO_MODE_DIRECT)
    {
        return;
    }

    for(unsigned int channel = 0; channel < zones.size(); channel++)
    {
        ApplyStaticColor(channel, colors[zones[channel].start_idx]);
    }
}

void RGBController_Maono::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_Maono::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_Maono::DeviceUpdateMode()
{
    const mode&  current = modes[active_mode];
    bool         block   = (info->protocol == MAONO_LIGHT_REGISTER_BLOCK);

    for(unsigned int channel = 0; channel < info->channels; channel++)
    {
        if(current.value == MAONO_MODE_OFF)
        {
            controller->SetPower(channel, false);
            continue;
        }

        controller->SetPower(channel, true);

        switch(current.value)
        {
            case MAONO_MODE_DIRECT:
                if(block)
                {
                    controller->SetEffect(channel, info->effect_static);
                }
                ApplyStaticColor(channel, colors[zones[channel].start_idx]);
                break;

            case MAONO_MODE_BREATHING:
                if(block)
                {
                    controller->SetEffect(channel, info->effect_breathing);
                    ApplyStaticColor(channel, current.colors[0]);
                }
                else
                {
                    ApplyPulseColor(channel, current.colors[0]);
                }
                break;

            case MAONO_MODE_COLOR_CYCLE:
                if(block)
                {
                    controller->SetEffect(channel, info->effect_cycle);
                }
                else
                {
                    controller->SetColorValue(channel, info->cycle_color_value);
                }
                break;

            case MAONO_MODE_RHYTHM:
                controller->SetEffect(channel, info->effect_rhythm);
                break;

            default:
                if(current.value >= MAONO_MODE_SPECIAL && current.value < MAONO_MODE_SPECIAL + info->num_specials)
                {
                    controller->SetColorValue(channel, info->specials[current.value - MAONO_MODE_SPECIAL].value);
                }
                break;
        }

        if(current.flags & MODE_FLAG_HAS_BRIGHTNESS)
        {
            controller->SetBrightness(channel, current.brightness);
        }
    }
}

void RGBController_Maono::DeviceSaveMode()
{
    /*-----------------------------------------------------*\
    | Maono Link has no save command; the microphones keep  |
    | the last values written                               |
    \*-----------------------------------------------------*/
}
