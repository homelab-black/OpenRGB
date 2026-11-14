/*---------------------------------------------------------*\
| RGBController_OnixLumiGPU.cpp                             |
|                                                           |
|   RGBController for ONIX LUMI Intel Arc B580              |
|                                                           |
|   Jan Dvořák                                  11 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_OnixLumiGPU.h"

/**------------------------------------------------------------------*\
    @name ONIX LUMI Intel Arc B580
    @category GPU
    @type I2C
    @save :x:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectOnixLumiGPUControllers
    @comment
\*-------------------------------------------------------------------*/

RGBController_OnixLumiGPU::RGBController_OnixLumiGPU(OnixLumiGPUController* controller_ptr)
{
    controller  = controller_ptr;

    name        = controller->GetDeviceName();
    vendor      = "ONIX";
    description = "ONIX LUMI Intel Arc B580 RGB Controller";
    location    = controller->GetDeviceLocation();

    type        = DEVICE_TYPE_GPU;

    mode Off;
    Off.name       = "Off";
    Off.value      = ONIX_LUMI_MODE_OFF;
    Off.flags      = 0;
    Off.color_mode = MODE_COLORS_NONE;
    modes.push_back(Off);

    mode Static;
    Static.name       = "Static";
    Static.value      = ONIX_LUMI_MODE_STATIC;
    Static.flags          = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Static.brightness     = 50;
    Static.brightness_min = 0;
    Static.brightness_max = 100;
    Static.color_mode     = MODE_COLORS_PER_LED;
    modes.push_back(Static);

    mode Rainbow;
    Rainbow.name       = "Rainbow";
    Rainbow.value      = ONIX_LUMI_MODE_RAINBOW;
    Rainbow.flags          = MODE_FLAG_HAS_BRIGHTNESS |
                               MODE_FLAG_HAS_SPEED |
                               MODE_FLAG_HAS_DIRECTION_LR;
    Rainbow.brightness     = 50;
    Rainbow.brightness_min = 0;
    Rainbow.brightness_max = 100;
    Rainbow.speed          = 5;
    Rainbow.speed_min      = 2;
    Rainbow.speed_max      = 13;
    Rainbow.direction      = MODE_DIRECTION_LEFT;
    Rainbow.color_mode     = MODE_COLORS_NONE;
    modes.push_back(Rainbow);

    mode Breathing;
    Breathing.name           = "Breathing";
    Breathing.value          = ONIX_LUMI_MODE_BREATHING;
    Breathing.flags          = MODE_FLAG_HAS_PER_LED_COLOR |
                               MODE_FLAG_HAS_BRIGHTNESS |
                               MODE_FLAG_HAS_SPEED;
    Breathing.brightness     = 50;
    Breathing.brightness_min = 0;
    Breathing.brightness_max = 100;
    Breathing.speed          = 8;
    Breathing.speed_min      = 0;
    Breathing.speed_max      = ONIX_LUMI_BREATHING_SPEED_MAX;
    Breathing.color_mode     = MODE_COLORS_PER_LED;
    modes.push_back(Breathing);
    mode Runway;
    Runway.name           = "Runway";
    Runway.value          = ONIX_LUMI_MODE_RUNWAY;
    Runway.flags          = MODE_FLAG_HAS_BRIGHTNESS |
                            MODE_FLAG_HAS_SPEED |
                            MODE_FLAG_HAS_DIRECTION_LR;
    Runway.brightness     = 50;
    Runway.brightness_min = 0;
    Runway.brightness_max = 100;
    Runway.speed          = 5;
    Runway.speed_min      = 2;
    Runway.speed_max      = 13;
    Runway.direction      = MODE_DIRECTION_LEFT;
    Runway.color_mode     = MODE_COLORS_NONE;
    modes.push_back(Runway);

    mode RunwayBounce;
    RunwayBounce.name           = "Runway Bounce";
    RunwayBounce.value          = ONIX_LUMI_MODE_RUNWAY_BOUNCE;
    RunwayBounce.flags          = MODE_FLAG_HAS_BRIGHTNESS |
                                  MODE_FLAG_HAS_SPEED;
    RunwayBounce.brightness     = 50;
    RunwayBounce.brightness_min = 0;
    RunwayBounce.brightness_max = 100;
    RunwayBounce.speed          = 5;
    RunwayBounce.speed_min      = 2;
    RunwayBounce.speed_max      = 13;
    RunwayBounce.color_mode     = MODE_COLORS_NONE;
    modes.push_back(RunwayBounce);
    mode ColorCycle;
    ColorCycle.name           = "Color Cycle";
    ColorCycle.value          = ONIX_LUMI_MODE_COLOR_CYCLE;
    ColorCycle.flags          = MODE_FLAG_HAS_BRIGHTNESS |
                                MODE_FLAG_HAS_SPEED;
    ColorCycle.brightness     = 50;
    ColorCycle.brightness_min = 0;
    ColorCycle.brightness_max = 100;
    ColorCycle.speed          = 5;
    ColorCycle.speed_min      = 2;
    ColorCycle.speed_max      = 13;
    ColorCycle.color_mode     = MODE_COLORS_NONE;
    modes.push_back(ColorCycle);
    mode ColorWipe;
    ColorWipe.name           = "Color Wipe";
    ColorWipe.value          = ONIX_LUMI_MODE_SERIAL;
    ColorWipe.flags          = MODE_FLAG_HAS_BRIGHTNESS |
                               MODE_FLAG_HAS_SPEED |
                               MODE_FLAG_HAS_DIRECTION_LR;
    ColorWipe.brightness     = 50;
    ColorWipe.brightness_min = 0;
    ColorWipe.brightness_max = 100;
    ColorWipe.speed          = 2;
    ColorWipe.speed_min      = 0;
    ColorWipe.speed_max      = 5;
    ColorWipe.direction      = MODE_DIRECTION_LEFT;
    ColorWipe.color_mode     = MODE_COLORS_NONE;
    modes.push_back(ColorWipe);
    mode BlockStacking;
    BlockStacking.name           = "Block Stacking";
    BlockStacking.value          = ONIX_LUMI_MODE_BLOCK_STACKING;
    BlockStacking.flags          = MODE_FLAG_HAS_BRIGHTNESS |
                                   MODE_FLAG_HAS_SPEED |
                                   MODE_FLAG_HAS_DIRECTION_LR;
    BlockStacking.brightness     = 50;
    BlockStacking.brightness_min = 0;
    BlockStacking.brightness_max = 100;
    BlockStacking.speed          = 5;
    BlockStacking.speed_min      = 2;
    BlockStacking.speed_max      = 13;
    BlockStacking.direction      = MODE_DIRECTION_LEFT;
    BlockStacking.color_mode     = MODE_COLORS_NONE;
    modes.push_back(BlockStacking);


    SetupZones();

    /*-----------------------------------------------------*\
    | Controller state is not queried during detection.      |
    | Use Rainbow as the initial OpenRGB fallback mode        |
    | without writing to the device.                          |
    \*-----------------------------------------------------*/
    for(unsigned int mode_idx = 0; mode_idx < modes.size(); mode_idx++)
    {
        if(modes[mode_idx].value == ONIX_LUMI_MODE_RAINBOW)
        {
            active_mode = mode_idx;
            break;
        }
    }
}

RGBController_OnixLumiGPU::~RGBController_OnixLumiGPU()
{
    Shutdown();

    delete controller;
}

void RGBController_OnixLumiGPU::SetupZones()
{
    zone new_zone;
    new_zone.name       = "LUMI";
    new_zone.type       = ZONE_TYPE_SINGLE;
    new_zone.leds_min   = 1;
    new_zone.leds_max   = 1;
    new_zone.leds_count = 1;

    led new_led;
    new_led.name = "GPU Lighting";

    zones.push_back(new_zone);
    leds.push_back(new_led);

    SetupColors();
}

void RGBController_OnixLumiGPU::DeviceUpdateLEDs()
{
    DeviceUpdateMode();
}

void RGBController_OnixLumiGPU::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_OnixLumiGPU::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_OnixLumiGPU::DeviceUpdateMode()
{
    switch(modes[active_mode].value)
    {
        case ONIX_LUMI_MODE_OFF:
            controller->SetOff();
            break;

        case ONIX_LUMI_MODE_STATIC:
        {
            RGBColor color = colors[0];

            controller->SetStatic(
                RGBGetRValue(color),
                RGBGetGValue(color),
                RGBGetBValue(color),
                modes[active_mode].brightness
            );
            break;
        }

        case ONIX_LUMI_MODE_RAINBOW:
            controller->SetRainbow(
                modes[active_mode].brightness,
                modes[active_mode].speed,
                modes[active_mode].direction
            );
            break;

        case ONIX_LUMI_MODE_BREATHING:
        {
            RGBColor color = colors[0];

            controller->SetBreathing(
                RGBGetRValue(color),
                RGBGetGValue(color),
                RGBGetBValue(color),
                modes[active_mode].brightness,
                modes[active_mode].speed
            );
            break;
        }
        case ONIX_LUMI_MODE_RUNWAY:
            controller->SetRunway(
                modes[active_mode].brightness,
                modes[active_mode].speed,
                modes[active_mode].direction,
                false
            );
            break;

        case ONIX_LUMI_MODE_RUNWAY_BOUNCE:
            controller->SetRunway(
                modes[active_mode].brightness,
                modes[active_mode].speed,
                MODE_DIRECTION_LEFT,
                true
            );
            break;
        case ONIX_LUMI_MODE_COLOR_CYCLE:
            controller->SetColorCycle(
                modes[active_mode].brightness,
                modes[active_mode].speed
            );
            break;
        case ONIX_LUMI_MODE_SERIAL:
            controller->SetSerial(
                modes[active_mode].brightness,
                modes[active_mode].speed,
                modes[active_mode].direction
            );
            break;
        case ONIX_LUMI_MODE_BLOCK_STACKING:
        {
            const unsigned char hardware_direction =
                (modes[active_mode].direction == MODE_DIRECTION_LEFT) ? 1 : 0;

            controller->SetBlockStacking(
                modes[active_mode].brightness,
                modes[active_mode].speed,
                hardware_direction
            );
            break;
        }


        default:
            break;
    }
}
