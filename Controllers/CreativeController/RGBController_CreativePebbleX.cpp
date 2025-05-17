/*---------------------------------------------------------*\
| RGBController_CreativePebbleX.cpp                         |
|                                                           |
|   RGBController for Creative Pebble X                     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "RGBController_CreativePebbleX.h"

/**------------------------------------------------------------------*\
    @name Creative Pebble X
    @category Speaker
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectCreativePebbleXDevice
    @comment
\*-------------------------------------------------------------------*/

int GetPebble_WaveDirection(int input);

RGBController_CreativePebbleX::RGBController_CreativePebbleX(CreativePebbleXController* controller_ptr)
{
    controller                  = controller_ptr;

    name                        = "Creative Pebble X Device";
    vendor                      = "Creative";
    type                        = DEVICE_TYPE_SPEAKER;
    description                 = "Creative Pebble X Device";
    location                    = controller->GetDeviceLocation();
    serial                      = "";

    mode Static;
    Static.name                 = "Direct";
    Static.value                = 0x03;
    Static.flags                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Static.color_mode           = MODE_COLORS_PER_LED;
    Static.brightness_min       = PEBBLE_X_BRIGHTNESS_MIN;
    Static.brightness_max       = PEBBLE_X_BRIGHTNESS_MAX;
    Static.brightness           = PEBBLE_X_BRIGHTNESS_MAX;
    modes.push_back(Static);

    mode Chasers;
    Chasers.name                 = "Chasers";
    Chasers.value                = 0x0B;
    Chasers.flags                = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Chasers.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    Chasers.colors_min           = 1;
    Chasers.colors_max           = 1;
    Chasers.brightness_min       = PEBBLE_X_BRIGHTNESS_MIN;
    Chasers.brightness_max       = PEBBLE_X_BRIGHTNESS_MAX;
    Chasers.brightness           = PEBBLE_X_BRIGHTNESS_MAX;
    Chasers.speed                = 0x00;
    Chasers.speed_max            = 0x06;
    Chasers.speed_min            = 0x00;
    Chasers.colors.resize(1);
    modes.push_back(Chasers);

    mode Aurora;
    Aurora.name                 = "Aurora";
    Aurora.value                = 0x08;
    Aurora.flags                = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Aurora.color_mode           = 0;
    Aurora.brightness_min       = PEBBLE_X_BRIGHTNESS_MIN;
    Aurora.brightness_max       = PEBBLE_X_BRIGHTNESS_MAX;
    Aurora.brightness           = PEBBLE_X_BRIGHTNESS_MAX;
    Aurora.speed                = 0x00;
    Aurora.speed_max            = 0x06;
    Aurora.speed_min            = 0x00;
    modes.push_back(Aurora);

    mode Peak_Meter;
    Peak_Meter.name                 = "Peak Meter";
    Peak_Meter.value                = 0x0a;
    Peak_Meter.flags                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_UD;
    Peak_Meter.color_mode           = MODE_COLORS_PER_LED;
    Peak_Meter.brightness_min       = PEBBLE_X_BRIGHTNESS_MIN;
    Peak_Meter.brightness_max       = PEBBLE_X_BRIGHTNESS_MAX;
    Peak_Meter.brightness           = PEBBLE_X_BRIGHTNESS_MAX;
    modes.push_back(Peak_Meter);

    mode Glowing;
    Glowing.name                 = "Glowing";
    Glowing.value                = 0x09;
    Glowing.flags                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Glowing.color_mode           = MODE_COLORS_PER_LED;
    Glowing.brightness_min       = PEBBLE_X_BRIGHTNESS_MIN;
    Glowing.brightness_max       = PEBBLE_X_BRIGHTNESS_MAX;
    Glowing.brightness           = PEBBLE_X_BRIGHTNESS_MAX;
    Glowing.speed                = 0x00;
    Glowing.speed_max            = 0x06;
    Glowing.speed_min            = 0x00;
    modes.push_back(Glowing);

    mode Wave;
    Wave.name                 = "Wave";
    Wave.value                = 0x04;
    Wave.flags                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD;
    Wave.color_mode           = MODE_COLORS_PER_LED;
    Wave.brightness_min       = PEBBLE_X_BRIGHTNESS_MIN;
    Wave.brightness_max       = PEBBLE_X_BRIGHTNESS_MAX;
    Wave.brightness           = PEBBLE_X_BRIGHTNESS_MAX;
    Wave.speed                = 0x00;
    Wave.speed_max            = 0x06;
    Wave.speed_min            = 0x00;
    modes.push_back(Wave);

    mode Cycle;
    Cycle.name                 = "Cycle";
    Cycle.value                = 0x01;
    Cycle.flags                = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Cycle.color_mode           = 0;
    Cycle.brightness_min       = PEBBLE_X_BRIGHTNESS_MIN;
    Cycle.brightness_max       = PEBBLE_X_BRIGHTNESS_MAX;
    Cycle.brightness           = PEBBLE_X_BRIGHTNESS_MAX;
    Cycle.speed                = 0x00;
    Cycle.speed_max            = 0x06;
    Cycle.speed_min            = 0x00;
    modes.push_back(Cycle);

    mode Morph;
    Morph.name                 = "Morph";
    Morph.value                = 0x07;
    Morph.flags                = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    Morph.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    Morph.color_mode           = 2;
    Morph.colors_min           = 2;
    Morph.colors_max           = 2;
    Morph.brightness_min       = PEBBLE_X_BRIGHTNESS_MIN;
    Morph.brightness_max       = PEBBLE_X_BRIGHTNESS_MAX;
    Morph.brightness           = PEBBLE_X_BRIGHTNESS_MAX;
    Morph.speed                = 0x00;
    Morph.speed_max            = 0x06;
    Morph.speed_min            = 0x00;
    Morph.colors.resize(2);
    modes.push_back(Morph);

    SetupZones();
}

RGBController_CreativePebbleX::~RGBController_CreativePebbleX()
{
    delete controller;
}

void RGBController_CreativePebbleX::SetupZones()
{
    zone pebble_speaker_zone;
    pebble_speaker_zone.name                  = "Speakers";
    pebble_speaker_zone.type                  = ZONE_TYPE_LINEAR;
    pebble_speaker_zone.leds_min              = 5;
    pebble_speaker_zone.leds_max              = 5;
    pebble_speaker_zone.leds_count            = 5;
    pebble_speaker_zone.matrix_map            = NULL;
    zones.push_back(pebble_speaker_zone);

    for (int i = 0; i < 5; ++i)
    {
        led led;
        led.name = "LED " + std::to_string(i + 1);
        leds.push_back(led);
    }

    SetupColors();
}

void RGBController_CreativePebbleX::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_CreativePebbleX::DeviceUpdateLEDs()
{
    controller->UpdateDeviceLEDs(colors);
}

void RGBController_CreativePebbleX::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_CreativePebbleX::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_CreativePebbleX::DeviceUpdateMode()
{
    controller->UpdateDeviceMode(modes[active_mode].value);
    if(modes[active_mode].flags & MODE_FLAG_HAS_BRIGHTNESS)
    {
        controller->SetBrightness(modes[active_mode].brightness);
    }
    if(modes[active_mode].flags & MODE_FLAG_HAS_SPEED)
    {
        controller->SetSpeed(modes[active_mode].speed);
    }
    if(modes[active_mode].flags & (MODE_FLAG_HAS_DIRECTION_UD | MODE_FLAG_HAS_DIRECTION_LR))
    {
        int waveDirection = GetPebble_WaveDirection(modes[active_mode].direction);
        controller->SetDirection(waveDirection);
    }
    if(modes[active_mode].color_mode & MODE_COLORS_MODE_SPECIFIC)
    {
        unsigned char red = RGBGetRValue(modes[active_mode].colors[0]);
        unsigned char grn = RGBGetGValue(modes[active_mode].colors[0]);
        unsigned char blu = RGBGetBValue(modes[active_mode].colors[0]);

        if(modes[active_mode].value == 0x07)
        {
            unsigned char red2 = RGBGetRValue(modes[active_mode].colors[1]);
            unsigned char grn2 = RGBGetGValue(modes[active_mode].colors[1]);
            unsigned char blu2 = RGBGetBValue(modes[active_mode].colors[1]);
            controller->UpdateMorphMode(red, grn, blu, red2, grn2, blu2);
        }
        else
        {
            controller->UpdateModeSpecificLEDs(red, grn, blu);
        }
    }
}


int GetPebble_WaveDirection(int input)
{
    switch(input)
    {
    case MODE_DIRECTION_LEFT:
        return(0x02);

    case MODE_DIRECTION_RIGHT:
        return(0x01);

    case MODE_DIRECTION_UP:
        return(0x04);

    case MODE_DIRECTION_DOWN:
        return(0x03);

    default:
        return(0x02);
    }
}

