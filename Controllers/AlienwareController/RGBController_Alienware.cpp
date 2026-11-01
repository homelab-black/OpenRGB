/*---------------------------------------------------------*\
| RGBController_Alienware.cpp                               |
|                                                           |
|   RGBController for Dell Alienware RGB USB controller     |
|                                                           |
|   Gabriel Marcano (gemarcano)                 19 Apr 2021 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <thread>
#include <chrono>
#include "RGBController_Alienware.h"

/*---------------------------------------------------------*\
| Aurora ACT1250 stadium is a vertical ring.  Each side has |
| five blocks of six LEDs, numbered down inside a block     |
| and up from block to block                                |
\*---------------------------------------------------------*/
#define NA  0xFFFFFFFF

static unsigned int act1250_stadium_map[30][4] =
{
    { NA, 59, 42, NA },
    { 58, NA, NA, 43 },
    { 57, NA, NA, 44 },
    { 56, NA, NA, 45 },
    { 55, NA, NA, 46 },
    { 54, NA, NA, 47 },
    { 53, NA, NA, 30 },
    { 52, NA, NA, 31 },
    { 51, NA, NA, 32 },
    { 50, NA, NA, 33 },
    { 49, NA, NA, 34 },
    { 48, NA, NA, 35 },
    { 41, NA, NA, 18 },
    { 40, NA, NA, 19 },
    { 39, NA, NA, 20 },
    { 38, NA, NA, 21 },
    { 37, NA, NA, 22 },
    { 36, NA, NA, 23 },
    { 29, NA, NA,  6 },
    { 28, NA, NA,  7 },
    { 27, NA, NA,  8 },
    { 26, NA, NA,  9 },
    { 25, NA, NA, 10 },
    { 24, NA, NA, 11 },
    { 17, NA, NA,  0 },
    { 16, NA, NA,  1 },
    { 15, NA, NA,  2 },
    { 14, NA, NA,  3 },
    { 13, NA, NA,  4 },
    { NA, 12,  5, NA }
};

/**------------------------------------------------------------------*\
    @name Alienware
    @category Keyboard
    @type USB
    @save :x:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectAlienwareControllers
    @comment Alienware Aurora ACT1250 also has Direct mode and a mode
    for each chassis zone.
\*-------------------------------------------------------------------*/

RGBController_Alienware::RGBController_Alienware(AlienwareController* controller_ptr)
{
    controller                  = controller_ptr;

    name                        = controller->GetDeviceName();
    vendor                      = "Alienware";
    type                        = controller->HasDirectMode() ? DEVICE_TYPE_CASE : DEVICE_TYPE_KEYBOARD;
    description                 = "Alienware USB Device";
    location                    = controller->GetDeviceLocation();
    serial                      = controller->GetSerialString();
    version                     = controller->GetFirmwareVersion();

    mode Color;
    Color.name                 = "Static";
    Color.value                = ALIENWARE_MODE_COLOR;
    Color.flags                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
    Color.color_mode           = MODE_COLORS_PER_LED;
    Color.colors_min           = 1;
    Color.colors_max           = 1;
    Color.brightness_min       = 100;
    Color.brightness_max       = 0;
    Color.brightness           = 0;
    modes.push_back(Color);

    mode Pulse;
    Pulse.name                 = "Flashing";
    Pulse.value                = ALIENWARE_MODE_PULSE;
    Pulse.flags                = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_MANUAL_SAVE;
    Pulse.color_mode           = MODE_COLORS_PER_LED;
    Pulse.colors_min           = 1;
    Pulse.colors_max           = 1;
    Pulse.speed_min            = ALIENWARE_TEMPO_MAX;
    Pulse.speed_max            = 50;
    Pulse.speed                = 150;
    Pulse.brightness_min       = 100;
    Pulse.brightness_max       = 0;
    Pulse.brightness           = 0;
    modes.push_back(Pulse);

    mode Morph;
    Morph.name                 = "Morph";
    Morph.value                = ALIENWARE_MODE_MORPH;
    Morph.flags                = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_MANUAL_SAVE;
    Morph.color_mode           = MODE_COLORS_MODE_SPECIFIC;
    Morph.colors_min           = 2;
    Morph.colors_max           = 2 * controller->GetZoneCount();
    Morph.colors.resize(2);
    Morph.colors[0]            = 0x0000FF;
    Morph.colors[1]            = 0xFF0000;
    Morph.speed_min            = ALIENWARE_TEMPO_MAX;
    Morph.speed_max            = 20;
    Morph.speed                = 100;
    Morph.brightness_min       = 100;
    Morph.brightness_max       = 0;
    Morph.brightness           = 0;
    modes.push_back(Morph);

    mode Spectrum;
    Spectrum.name              = "Spectrum Cycle";
    Spectrum.value             = ALIENWARE_MODE_SPECTRUM;
    Spectrum.flags             = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_MANUAL_SAVE;
    Spectrum.color_mode        = MODE_COLORS_NONE;
    Spectrum.speed_min         = ALIENWARE_TEMPO_MAX;
    Spectrum.speed_max         = ALIENWARE_TEMPO_SPECTRUM;
    Spectrum.speed             = 40;
    Spectrum.brightness_min    = 100;
    Spectrum.brightness_max    = 0;
    Spectrum.brightness        = 0;
    modes.push_back(Spectrum);

    mode Rainbow;
    Rainbow.name               = "Rainbow Wave";
    Rainbow.value              = ALIENWARE_MODE_RAINBOW;
    Rainbow.flags              = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_MANUAL_SAVE;
    Rainbow.color_mode         = MODE_COLORS_NONE;
    Rainbow.speed_min          = ALIENWARE_TEMPO_MAX;
    Rainbow.speed_max          = ALIENWARE_TEMPO_SPECTRUM;
    Rainbow.speed              = 40;
    Rainbow.direction          = MODE_DIRECTION_RIGHT;
    Rainbow.brightness_min     = 100;
    Rainbow.brightness_max     = 0;
    Rainbow.brightness         = 0;
    modes.push_back(Rainbow);

    mode Breathing;
    Breathing.name             = "Breathing";
    Breathing.value            = ALIENWARE_MODE_BREATHING;
    Breathing.flags            = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_MANUAL_SAVE;
    Breathing.color_mode       = MODE_COLORS_PER_LED;
    Breathing.colors_min       = 1;
    Breathing.colors_max       = 1;
    Breathing.speed_min        = ALIENWARE_TEMPO_MAX;
    Breathing.speed_max        = 50;
    Breathing.speed            = 150;
    Breathing.brightness_min   = 100;
    Breathing.brightness_max   = 0;
    Breathing.brightness       = 0;
    modes.push_back(Breathing);

    if(controller->HasDirectMode())
    {
        mode Direct;
        Direct.name            = "Direct";
        Direct.value           = ALIENWARE_MODE_DIRECT;
        Direct.flags           = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_REQUIRES_ENTIRE_DEVICE;
        Direct.color_mode      = MODE_COLORS_PER_LED;
        modes.push_back(Direct);
    }

    SetupZones();

    /*-----------------------------------------------------*\
    | Series are independent, so each chassis zone can play |
    | its own mode, except for direct mode                  |
    \*-----------------------------------------------------*/
    if(controller->HasDirectMode())
    {
        for(unsigned int zone_idx = 0; zone_idx < zones.size(); zone_idx++)
        {
            for(unsigned int mode_idx = 0; mode_idx < modes.size(); mode_idx++)
            {
                if(modes[mode_idx].value != ALIENWARE_MODE_DIRECT)
                {
                    zones[zone_idx].modes.push_back(modes[mode_idx]);
                }
            }
        }
    }

    controller->UpdateDim();
}

RGBController_Alienware::~RGBController_Alienware()
{
    Shutdown();

    delete controller;
}

void RGBController_Alienware::SetupZones()
{
    /*-------------------------------------------------*\
    | Set zones and leds                                |
    \*-------------------------------------------------*/
    std::vector<const char*> zone_names = controller->GetZoneNames();

    if(controller->HasDirectMode())
    {
        const char*  chassis_names[]  = { "Stadium", "Power Button", "RGB System Fan", "Liquid Cooling" };
        unsigned int chassis_counts[] = { 60, 3, 1, 1 };

        for(unsigned int zone_idx = 0; zone_idx < 4; zone_idx++)
        {
            zone new_zone;

            new_zone.name       = chassis_names[zone_idx];
            new_zone.type       = (chassis_counts[zone_idx] > 1) ? ZONE_TYPE_LINEAR : ZONE_TYPE_SINGLE;
            new_zone.leds_min   = chassis_counts[zone_idx];
            new_zone.leds_max   = chassis_counts[zone_idx];
            new_zone.leds_count = chassis_counts[zone_idx];

            if(zone_idx == 0)
            {
                new_zone.type   = ZONE_TYPE_MATRIX;
                new_zone.matrix_map.Set(30, 4, (unsigned int *)act1250_stadium_map);
            }

            zones.emplace_back(new_zone);
        }

        for(unsigned int led_idx = 0; led_idx < controller->GetZoneCount(); led_idx++)
        {
            led new_led;

            new_led.name    = zone_names[led_idx];

            leds.emplace_back(new_led);
        }
    }
    else
    {
        for(unsigned int zone_idx = 0; zone_idx < controller->GetZoneCount(); zone_idx++)
        {
            zone new_zone;

            new_zone.name       = zone_names[zone_idx];
            new_zone.type       = ZONE_TYPE_SINGLE;
            new_zone.leds_min   = 1;
            new_zone.leds_max   = 1;
            new_zone.leds_count = 1;

            zones.emplace_back(new_zone);
        }

        for(unsigned int led_idx = 0; led_idx < zones.size(); led_idx++)
        {
            led new_led;

            new_led.name        = zones[led_idx].name + std::string(" LED");

            leds.emplace_back(new_led);
        }
    }

    SetupColors();
}

void RGBController_Alienware::DeviceUpdateLEDs()
{
    DeviceUpdateMode();
}

void RGBController_Alienware::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_Alienware::DeviceUpdateSingleLED(int led)
{
    DeviceUpdateZoneLEDs(led);
}

static bool modes_eq(const mode& mode1, const mode& mode2)
{
    return( ( mode1.name           == mode2.name       )
         && ( mode1.value          == mode2.value      )
         && ( mode1.flags          == mode2.flags      )
         && ( mode1.speed_min      == mode2.speed_min  )
         && ( mode1.speed_max      == mode2.speed_max  )
         && ( mode1.colors_min     == mode2.colors_min )
         && ( mode1.colors_max     == mode2.colors_max )
         && ( mode1.speed          == mode2.speed      )
         && ( mode1.direction      == mode2.direction  )
         && ( mode1.color_mode     == mode2.color_mode )
         && ( mode1.colors         == mode2.colors     )
         && ( mode1.brightness     == mode2.brightness )
         && ( mode1.brightness_min == mode2.brightness_min )
         && ( mode1.brightness_max == mode2.brightness_max ) );
}

void RGBController_Alienware::SetLEDMode(unsigned int led_idx, const mode& led_mode)
{
    /*-----------------------------------------------------*\
    | The same settings as in DeviceUpdateMode, for one LED |
    \*-----------------------------------------------------*/
    uint8_t  led      = (uint8_t)led_idx;
    uint16_t period   = 0x07D0;

    controller->SetMode(led, led_mode.value);
    controller->SetDirection(led, led_mode.direction);
    controller->SetDim(led, led_mode.brightness);

    switch(led_mode.value)
    {
        case ALIENWARE_MODE_COLOR:
            controller->SetPeriod(led, period);
            controller->SetColor(led, colors[led_idx]);
            controller->SetTempo(led, ALIENWARE_TEMPO_MAX);
            break;

        case ALIENWARE_MODE_PULSE:
            period = std::max<uint16_t>(500, (uint16_t)(led_mode.speed * 10));
            controller->SetPeriod(led, period);
            controller->SetColor(led, colors[led_idx]);
            controller->SetTempo(led, led_mode.speed);
            break;

        case ALIENWARE_MODE_MORPH:
            period = std::max<uint16_t>(400, (uint16_t)(led_mode.speed * 10));
            controller->SetPeriod(led, period);
            controller->SetColor(led, (led_mode.colors.size() >= 1) ? led_mode.colors[0] : 0x0000FF,
                                      (led_mode.colors.size() >= 2) ? led_mode.colors[1] : 0xFF0000);
            controller->SetTempo(led, led_mode.speed);
            break;

        case ALIENWARE_MODE_SPECTRUM:
        case ALIENWARE_MODE_RAINBOW:
            controller->SetPeriod(led, ALIENWARE_DURATION_SPECTRUM);
            controller->SetTempo(led, led_mode.speed);
            break;

        case ALIENWARE_MODE_BREATHING:
            period = std::max<uint16_t>(500, (uint16_t)(led_mode.speed * 10));
            controller->SetPeriod(led, period);
            controller->SetColor(led, colors[led_idx], 0x0);
            controller->SetTempo(led, led_mode.speed);
            break;
    }
}

void RGBController_Alienware::UpdateChassisMode()
{
    if(modes[active_mode].value == ALIENWARE_MODE_DIRECT)
    {
        controller->SetDirectColors(colors);
        return;
    }

    controller->LeaveDirectMode();

    /*-----------------------------------------------------*\
    | A zone with no own mode follows the device mode.  The |
    | controller keeps one animation, so it is sent whole   |
    \*-----------------------------------------------------*/
    for(unsigned int zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        bool        own_mode  = (zones[zone_idx].active_mode >= 0) && (zones[zone_idx].active_mode < (int)zones[zone_idx].modes.size());
        const mode& zone_mode = own_mode ? zones[zone_idx].modes[zones[zone_idx].active_mode] : modes[active_mode];

        for(unsigned int led_idx = zones[zone_idx].start_idx; led_idx < zones[zone_idx].start_idx + zones[zone_idx].leds_count; led_idx++)
        {
            SetLEDMode(led_idx, zone_mode);
        }
    }

    controller->UpdateController();
}

void RGBController_Alienware::DeviceUpdateZoneMode(int /*zone*/)
{
    UpdateChassisMode();
}

void RGBController_Alienware::DeviceUpdateMode()
{
    if(controller->HasDirectMode())
    {
        UpdateChassisMode();
        return;
    }

    /*---------------------------------------------------------*\
    | Copy mode to get the current state-- this is racy, as the |
    | UI thread can be actively modifying this variable         |
    \*---------------------------------------------------------*/
    int  current_mode_idx = active_mode;
    mode current_mode     = modes[current_mode_idx];

    bool done             = false;

    while(!done)
    {
        /*-----------------------------------------------------*\
        | Setup state per zone                                  |
        \*-----------------------------------------------------*/
        for(uint8_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
        {
            zone current_zone = zones[zone_idx];

            /*-------------------------------------------------*\
            | Some modes use 2000ms (0x07D0) for their duration,|
            | per traces                                        |
            \*-------------------------------------------------*/
            uint16_t period = 0x07d0;

            controller->SetMode(zone_idx, current_mode.value);
            controller->SetDirection(zone_idx, current_mode.direction);

            switch(current_mode_idx)
            {
                case ALIENWARE_MODE_COLOR:
                    controller->SetPeriod(zone_idx, period);
                    controller->SetColor( zone_idx, colors[current_zone.start_idx]);
                    controller->SetTempo( zone_idx, ALIENWARE_TEMPO_MAX);
                    controller->SetDim(   zone_idx, modes[current_mode_idx].brightness);
                    break;

                case ALIENWARE_MODE_PULSE:
                    period = std::max<uint16_t>(500, (uint16_t)(current_mode.speed * 10));
                    controller->SetPeriod(zone_idx, period);
                    controller->SetColor( zone_idx, colors[current_zone.start_idx]);
                    controller->SetTempo( zone_idx, current_mode.speed);
                    controller->SetDim(   zone_idx, modes[current_mode_idx].brightness);
                    break;

                case ALIENWARE_MODE_MORPH:
                    {
                        RGBColor c1 = (current_mode.colors.size() >= 1) ? current_mode.colors[0] : 0x0000FF;
                        RGBColor c2 = (current_mode.colors.size() >= 2) ? current_mode.colors[1] : 0xFF0000;

                        if(current_mode.colors.size() >= (std::size_t)((zone_idx + 1) * 2))
                        {
                            c1 = current_mode.colors[zone_idx * 2];
                            c2 = current_mode.colors[(zone_idx * 2) + 1];
                        }

                        period = std::max<uint16_t>(400, (uint16_t)(current_mode.speed * 10));
                        controller->SetPeriod(zone_idx, period);
                        controller->SetColor( zone_idx, c1, c2);
                        controller->SetTempo( zone_idx, current_mode.speed);
                        controller->SetDim(   zone_idx, modes[current_mode_idx].brightness);
                    }
                    break;

                case ALIENWARE_MODE_SPECTRUM:
                case ALIENWARE_MODE_RAINBOW:
                    controller->SetPeriod(zone_idx, ALIENWARE_DURATION_SPECTRUM);
                    controller->SetTempo( zone_idx, current_mode.speed);
                    controller->SetDim(   zone_idx, modes[current_mode_idx].brightness);
                    break;

                case ALIENWARE_MODE_BREATHING:
                    period = std::max<uint16_t>(500, (uint16_t)(current_mode.speed * 10));
                    controller->SetPeriod(zone_idx, period);
                    controller->SetColor( zone_idx, colors[current_zone.start_idx], 0x0);
                    controller->SetTempo( zone_idx, current_mode.speed);
                    controller->SetDim(   zone_idx, modes[current_mode_idx].brightness);
                    break;
            }
        }

        /*-----------------------------------------------------*\
        | Due to rate-limiting, this can take more than one     |
        | second to execute                                     |
        \*-----------------------------------------------------*/
        controller->UpdateController();

        /*-----------------------------------------------------*\
        | Re-run update if there's anything that's changed from |
        | under us...                                           |
        \*-----------------------------------------------------*/
        int  new_current_mode_idx = active_mode;
        mode new_current_mode     = modes[current_mode_idx];

        done                      = (current_mode_idx == new_current_mode_idx && modes_eq(new_current_mode, current_mode));
        current_mode_idx          = new_current_mode_idx;
        current_mode              = new_current_mode;
    }
}

void RGBController_Alienware::DeviceSaveMode()
{
    controller->SaveController();
}

