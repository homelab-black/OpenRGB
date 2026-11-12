/*---------------------------------------------------------*\
| RGBController_HyperXPulsefireHaste2.cpp                   |
|                                                           |
|   RGBController for HyperX Pulsefire Haste 2 Wireless     |
|                                                           |
|   Plamen Marinov (PMarinov1994)               14 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_HyperXPulsefireHaste2.h"

/**------------------------------------------------------------------*\
    @name HyperX Pulsefire Haste 2 Wireless
    @category Mouse
    @type USB
    @save :white_check_mark:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectHyperXPulsefireHaste2Controllers
    @comment Controlled through the wireless dongle.  Direct mode
             re-sends the current color every 500ms because the
             mouse returns to its own effect when frames stop.
\*-------------------------------------------------------------------*/

RGBController_HyperXPulsefireHaste2::RGBController_HyperXPulsefireHaste2(HyperXPulsefireHaste2Controller* controller_ptr)
{
    controller                      = controller_ptr;

    name                            = controller->GetNameString();
    vendor                          = "HyperX";
    type                            = DEVICE_TYPE_MOUSE;
    description                     = "HyperX Pulsefire Haste 2 Wireless Device";
    location                        = controller->GetDeviceLocation();
    serial                          = controller->GetSerialString();

    mode Direct;
    Direct.name                     = "Direct";
    Direct.value                    = HYPERX_PULSEFIRE_HASTE_2_MODE_DIRECT;
    Direct.flags                    = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
    Direct.color_mode               = MODE_COLORS_PER_LED;
    Direct.brightness_min           = HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MIN;
    Direct.brightness_max           = HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MAX;
    Direct.brightness               = HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MAX;
    modes.push_back(Direct);

    mode Static;
    Static.name                     = "Static";
    Static.value                    = HYPERX_PULSEFIRE_HASTE_2_MODE_STATIC;
    Static.flags                    = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
    Static.color_mode               = MODE_COLORS_MODE_SPECIFIC;
    Static.colors_min               = 1;
    Static.colors_max               = 1;
    Static.colors.resize(1);
    Static.brightness_min           = HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MIN;
    Static.brightness_max           = HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MAX;
    Static.brightness               = HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MAX;
    modes.push_back(Static);

    mode Breathing;
    Breathing.name                  = "Breathing";
    Breathing.value                 = HYPERX_PULSEFIRE_HASTE_2_MODE_BREATHING;
    Breathing.flags                 = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
    Breathing.color_mode            = MODE_COLORS_MODE_SPECIFIC;
    Breathing.colors_min            = 1;
    Breathing.colors_max            = 1;
    Breathing.colors.resize(1);
    Breathing.speed_min             = HYPERX_PULSEFIRE_HASTE_2_SPEED_SLOWEST;
    Breathing.speed_max             = HYPERX_PULSEFIRE_HASTE_2_SPEED_FASTEST;
    Breathing.speed                 = HYPERX_PULSEFIRE_HASTE_2_SPEED_DEFAULT;
    Breathing.brightness_min        = HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MIN;
    Breathing.brightness_max        = HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MAX;
    Breathing.brightness            = HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MAX;
    modes.push_back(Breathing);

    mode SpectrumCycle;
    SpectrumCycle.name              = "Spectrum Cycle";
    SpectrumCycle.value             = HYPERX_PULSEFIRE_HASTE_2_MODE_SPECTRUM_CYCLE;
    SpectrumCycle.flags             = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
    SpectrumCycle.color_mode        = MODE_COLORS_NONE;
    SpectrumCycle.speed_min         = HYPERX_PULSEFIRE_HASTE_2_SPEED_SLOWEST;
    SpectrumCycle.speed_max         = HYPERX_PULSEFIRE_HASTE_2_SPEED_FASTEST;
    SpectrumCycle.speed             = HYPERX_PULSEFIRE_HASTE_2_SPEED_DEFAULT;
    SpectrumCycle.brightness_min    = HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MIN;
    SpectrumCycle.brightness_max    = HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MAX;
    SpectrumCycle.brightness        = HYPERX_PULSEFIRE_HASTE_2_BRIGHTNESS_MAX;
    modes.push_back(SpectrumCycle);

    mode Off;
    Off.name                        = "Off";
    Off.value                       = HYPERX_PULSEFIRE_HASTE_2_MODE_OFF;
    Off.flags                       = MODE_FLAG_MANUAL_SAVE;
    Off.color_mode                  = MODE_COLORS_NONE;
    modes.push_back(Off);

    SetupZones();
}

RGBController_HyperXPulsefireHaste2::~RGBController_HyperXPulsefireHaste2()
{
    Shutdown();

    delete controller;
}

void RGBController_HyperXPulsefireHaste2::SetupZones()
{
    zone scroll_wheel_zone;
    scroll_wheel_zone.name          = "Scroll Wheel";
    scroll_wheel_zone.type          = ZONE_TYPE_SINGLE;
    scroll_wheel_zone.leds_min      = 1;
    scroll_wheel_zone.leds_max      = 1;
    scroll_wheel_zone.leds_count    = 1;
    zones.push_back(scroll_wheel_zone);

    led scroll_wheel_led;
    scroll_wheel_led.name           = "Scroll Wheel";
    leds.push_back(scroll_wheel_led);

    SetupColors();
}

void RGBController_HyperXPulsefireHaste2::DeviceUpdateLEDs()
{
    if(modes[active_mode].value == HYPERX_PULSEFIRE_HASTE_2_MODE_DIRECT)
    {
        controller->SetDirectColor(colors[0]);
    }
}

void RGBController_HyperXPulsefireHaste2::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_HyperXPulsefireHaste2::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_HyperXPulsefireHaste2::DeviceUpdateMode()
{
    controller->SetMode(modes[active_mode].value, modes[active_mode].speed, GetActiveColor(), modes[active_mode].brightness);
}

void RGBController_HyperXPulsefireHaste2::DeviceSaveMode()
{
    controller->SaveMode(modes[active_mode].value, modes[active_mode].speed, GetActiveColor(), modes[active_mode].brightness);
}

RGBColor RGBController_HyperXPulsefireHaste2::GetActiveColor()
{
    if(modes[active_mode].color_mode == MODE_COLORS_MODE_SPECIFIC)
    {
        return(modes[active_mode].colors[0]);
    }

    return(colors[0]);
}
