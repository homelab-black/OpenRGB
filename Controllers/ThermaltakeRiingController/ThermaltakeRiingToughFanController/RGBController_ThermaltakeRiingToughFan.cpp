/*---------------------------------------------------------*\
| RGBController_ThermaltakeRiingToughFan.cpp                |
|                                                           |
|   RGBController for Thermaltake ToughFan 14 RGB           |
|                                                           |
|   Omar Jaber (Bewinxed)                       18 Mar 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_ThermaltakeRiingToughFan.h"

/**------------------------------------------------------------------*\
    @name Thermaltake ToughFan 14 RGB
    @category Cooler
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectThermaltakeRiingToughFanControllers
    @comment The ToughFan 14 RGB uses a 4th-gen Thermaltake controller
        hub (VID 0x264A, PID 0x232B) with Riing Quad-compatible
        protocol. Each fan has 24 addressable LEDs in GRB order.
        The controller auto-detects which channels have fans
        connected via RPM polling at initialization.
\*-------------------------------------------------------------------*/

RGBController_ThermaltakeRiingToughFan::RGBController_ThermaltakeRiingToughFan(ThermaltakeRiingToughFanController* controller_ptr)
{
    controller  = controller_ptr;

    name        = controller->GetDeviceName();
    vendor      = "Thermaltake";
    type        = DEVICE_TYPE_COOLER;
    description = "Thermaltake ToughFan 14 RGB Device";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerial();
    version     = controller->GetFirmwareVersion();

    mode Direct;
    Direct.name       = "Direct";
    Direct.value      = THERMALTAKE_TOUGHFAN_MODE_DIRECT;
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.speed_min  = 0;
    Direct.speed_max  = 0;
    Direct.speed      = 0;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_ThermaltakeRiingToughFan::~RGBController_ThermaltakeRiingToughFan()
{
    delete controller;
}

void RGBController_ThermaltakeRiingToughFan::SetupZones()
{
    /*-------------------------------------------------*\
    | Only set LED count on the first run               |
    \*-------------------------------------------------*/
    bool first_run = false;

    if(zones.size() == 0)
    {
        first_run = true;
    }

    /*-------------------------------------------------*\
    | Clear any existing color/LED configuration        |
    \*-------------------------------------------------*/
    leds.clear();
    colors.clear();
    leds_channel.clear();
    zones.resize(THERMALTAKE_TOUGHFAN_NUM_CHANNELS);

    unsigned int leds_per_fan = controller->GetLEDsPerFan();

    /*-------------------------------------------------*\
    | Set zones and leds                                |
    \*-------------------------------------------------*/
    for(unsigned int channel_idx = 0; channel_idx < THERMALTAKE_TOUGHFAN_NUM_CHANNELS; channel_idx++)
    {
        char ch_idx_string[2];
        snprintf(ch_idx_string, 2, "%d", channel_idx + 1);

        zones[channel_idx].name     = "ToughFan Channel ";
        zones[channel_idx].name.append(ch_idx_string);
        zones[channel_idx].type     = ZONE_TYPE_LINEAR;

        /*-------------------------------------------------*\
        | The maximum number of colors that would fit in the|
        | ToughFan protocol is 62 ((193-5)/3)               |
        \*-------------------------------------------------*/
        zones[channel_idx].leds_min   = 0;
        zones[channel_idx].leds_max   = 60;

        if(first_run)
        {
            /*---------------------------------------------*\
            | Auto-detect: active channels default to 24    |
            | LEDs per fan, inactive channels default to 0. |
            | User can still resize in the UI.              |
            \*---------------------------------------------*/
            if(controller->IsChannelActive(channel_idx))
            {
                zones[channel_idx].leds_count = leds_per_fan;
            }
            else
            {
                zones[channel_idx].leds_count = 0;
            }
        }

        zones[channel_idx].matrix_map = NULL;

        for(unsigned int led_ch_idx = 0; led_ch_idx < zones[channel_idx].leds_count; led_ch_idx++)
        {
            char led_idx_string[4];
            snprintf(led_idx_string, 4, "%d", led_ch_idx + 1);

            led new_led;
            new_led.name = "ToughFan Channel ";
            new_led.name.append(ch_idx_string);
            new_led.name.append(", LED ");
            new_led.name.append(led_idx_string);

            leds.push_back(new_led);
            leds_channel.push_back(channel_idx);
        }
    }

    SetupColors();
}

void RGBController_ThermaltakeRiingToughFan::ResizeZone(int zone, int new_size)
{
    if((std::size_t)zone >= zones.size())
    {
        return;
    }

    if(((unsigned int)new_size >= zones[zone].leds_min) && ((unsigned int)new_size <= zones[zone].leds_max))
    {
        zones[zone].leds_count = new_size;

        SetupZones();
    }
}

void RGBController_ThermaltakeRiingToughFan::DeviceUpdateLEDs()
{
    for(std::size_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        if(zones[zone_idx].leds_count > 0)
        {
            controller->SetChannelLEDs((unsigned char)zone_idx, zones[zone_idx].colors, zones[zone_idx].leds_count);
        }
    }
}

void RGBController_ThermaltakeRiingToughFan::UpdateZoneLEDs(int zone)
{
    if(zones[zone].leds_count > 0)
    {
        controller->SetChannelLEDs(zone, zones[zone].colors, zones[zone].leds_count);
    }
}

void RGBController_ThermaltakeRiingToughFan::UpdateSingleLED(int led)
{
    unsigned int channel = leds_channel[led];

    if(zones[channel].leds_count > 0)
    {
        controller->SetChannelLEDs(channel, zones[channel].colors, zones[channel].leds_count);
    }
}

void RGBController_ThermaltakeRiingToughFan::DeviceUpdateMode()
{
    for(std::size_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        controller->SetMode(modes[active_mode].value, modes[active_mode].speed);
        controller->SetChannelLEDs((unsigned char)zone_idx, zones[zone_idx].colors, zones[zone_idx].leds_count);
    }
}
