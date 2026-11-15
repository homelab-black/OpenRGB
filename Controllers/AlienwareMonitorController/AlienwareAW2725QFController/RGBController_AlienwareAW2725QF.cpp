/*---------------------------------------------------------*\
| RGBController_AlienwareAW2725QF.cpp                       |
|                                                           |
|   RGBController for the Alienware AW2725QF monitor        |
|                                                           |
|   Ferréol DUBOIS COLI (Fefe_du_973)           23 Jan 2025 |
|   Jingcheng Tang (at1289)                     08 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_AlienwareAW2725QF.h"

/**------------------------------------------------------------------*\
    @name AW2725QF
    @category Monitor
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectAlienwareAW2725QFControllers
    @comment
\*-------------------------------------------------------------------*/

RGBController_AlienwareAW2725QF::RGBController_AlienwareAW2725QF(AlienwareAW2725QFController* controller_ptr)
{
    controller              = controller_ptr;

    name                    = "Alienware AW2725QF";
    vendor                  = "Alienware";
    type                    = DEVICE_TYPE_MONITOR;
    description             = "Alienware AW2725QF Monitor Device";
    location                = controller->GetLocation();
    serial                  = controller->GetSerialString();

    active_mode             = 0;

    SetupZones();
    SetupModes();
}

RGBController_AlienwareAW2725QF::~RGBController_AlienwareAW2725QF()
{
    Shutdown();

    delete controller;
}

void RGBController_AlienwareAW2725QF::SetupZones()
{
    zone Logo;
    Logo.name               = "Logo";
    Logo.type               = ZONE_TYPE_SINGLE;
    Logo.leds_min           = 1;
    Logo.leds_max           = 1;
    Logo.leds_count         = 1;
    zones.push_back(Logo);

    led Logo_LED;
    Logo_LED.name           = "Logo";
    Logo_LED.value          = AW2725QF_ZONE_LOGO;
    leds.push_back(Logo_LED);

    zone Number;
    Number.name             = "Number";
    Number.type             = ZONE_TYPE_SINGLE;
    Number.leds_min         = 1;
    Number.leds_max         = 1;
    Number.leds_count       = 1;
    zones.push_back(Number);

    led Number_LED;
    Number_LED.name         = "Number";
    Number_LED.value        = AW2725QF_ZONE_NUMBER;
    leds.push_back(Number_LED);

    zone PowerButton;
    PowerButton.name        = "Power Button";
    PowerButton.type        = ZONE_TYPE_SINGLE;
    PowerButton.leds_min    = 1;
    PowerButton.leds_max    = 1;
    PowerButton.leds_count  = 1;
    zones.push_back(PowerButton);

    led PowerButton_LED;
    PowerButton_LED.name    = "Power Button";
    PowerButton_LED.value   = AW2725QF_ZONE_POWER;
    leds.push_back(PowerButton_LED);

    SetupColors();
}

void RGBController_AlienwareAW2725QF::SetupModes()
{
    mode Direct;
    Direct.name             = "Direct";
    Direct.value            = AW2725QF_MODE_DIRECT;
    Direct.flags            = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Direct.color_mode       = MODE_COLORS_PER_LED;
    Direct.brightness_min   = 0;
    Direct.brightness_max   = AW2725QF_BRIGHTNESS_MAX;
    Direct.brightness       = AW2725QF_BRIGHTNESS_MAX;
    modes.push_back(Direct);

    mode Pulse;
    Pulse.name              = "Pulse";
    Pulse.value             = AW2725QF_MODE_PULSE;
    Pulse.flags             = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;
    Pulse.color_mode        = MODE_COLORS_MODE_SPECIFIC;
    Pulse.colors_min        = 1;
    Pulse.colors_max        = 1;
    Pulse.colors.resize(1);
    Pulse.brightness_min    = 0;
    Pulse.brightness_max    = AW2725QF_BRIGHTNESS_MAX;
    Pulse.brightness        = AW2725QF_BRIGHTNESS_MAX;
    Pulse.speed_min         = AW2725QF_SPEED_SLOW;
    Pulse.speed_max         = AW2725QF_SPEED_FAST;
    Pulse.speed             = AW2725QF_SPEED_MEDIUM;
    modes.push_back(Pulse);

    mode Morph              = Pulse;
    Morph.name              = "Morph";
    Morph.value             = AW2725QF_MODE_MORPH;
    Morph.colors_min        = 2;
    Morph.colors_max        = 2;
    Morph.colors.resize(2);
    modes.push_back(Morph);

    mode Breathing          = Pulse;
    Breathing.name          = "Breathing";
    Breathing.value         = AW2725QF_MODE_BREATHING;
    modes.push_back(Breathing);

    /*-------------------------------------------------*\
    | Spectrum mode only applies to all zones.          |
    | It overwrites other zones even if you specify a   |
    | zone in command. Also, if you try to change a     |
    | zone out of spectrum mode it stops all other zone |
    | as well.                                          |
    \*-------------------------------------------------*/
    mode Spectrum;
    Spectrum.name           = "Spectrum Cycle";
    Spectrum.value          = AW2725QF_MODE_SPECTRUM;
    Spectrum.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_REQUIRES_ENTIRE_DEVICE;
    Spectrum.color_mode     = MODE_COLORS_NONE;
    Spectrum.speed_min      = AW2725QF_SPEED_SLOW;
    Spectrum.speed_max      = AW2725QF_SPEED_FAST;
    Spectrum.speed          = AW2725QF_SPEED_MEDIUM;
    modes.push_back(Spectrum);

    for(unsigned int zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        for(unsigned int mode_idx = 0; mode_idx < modes.size(); mode_idx++)
        {
            if(!(modes[mode_idx].flags & MODE_FLAG_REQUIRES_ENTIRE_DEVICE))
            {
                zones[zone_idx].modes.push_back(modes[mode_idx]);
            }
        }
        zones[zone_idx].active_mode = -1;
    }
}

void RGBController_AlienwareAW2725QF::UpdateMode()
{
    /*-----------------------------------------------------*\
    | Make sure Spectrum or All Device Direct mode and zone |
    | specific override works happily together.             |
    | Apply a all device mode clears zone specific modes.   |
    \*-----------------------------------------------------*/
    AccessMutex.lock();
    if(active_mode >= 0 && active_mode < (int)modes.size()
    && (modes[active_mode].value == AW2725QF_MODE_DIRECT
     || (modes[active_mode].flags & MODE_FLAG_REQUIRES_ENTIRE_DEVICE)))
    {
        for(unsigned int zone_idx = 0; zone_idx < zones.size(); zone_idx++)
        {
            zones[zone_idx].active_mode = -1;
        }
    }
    AccessMutex.unlock();

    RGBController::UpdateMode();
}

const mode& RGBController_AlienwareAW2725QF::GetEffectiveMode(int zone) const
{
    int zone_mode = zones[zone].active_mode;
    if(zone_mode >= 0 && zone_mode < (int)zones[zone].modes.size())
    {
        return zones[zone].modes[zone_mode];
    }
    return modes[active_mode];
}

void RGBController_AlienwareAW2725QF::DeviceUpdateLEDs()
{
    if(modes[active_mode].flags & MODE_FLAG_REQUIRES_ENTIRE_DEVICE)
    {
        return;
    }

    /*-----------------------------------------------------*\
    | Combine writes to the single All Device zone if all   |
    | values match. Makes it faster because we have to do   |
    | authentications per command to the device.            |
    \*-----------------------------------------------------*/
    const mode& logo_mode   = GetEffectiveMode(0);
    const mode& number_mode = GetEffectiveMode(1);
    const mode& power_mode  = GetEffectiveMode(2);
    if(logo_mode.value == AW2725QF_MODE_DIRECT && number_mode.value == AW2725QF_MODE_DIRECT && power_mode.value == AW2725QF_MODE_DIRECT
    && logo_mode.brightness == number_mode.brightness && number_mode.brightness == power_mode.brightness
    && colors[0] == colors[1] && colors[1] == colors[2])
    {
        controller->SetMode(AW2725QF_MODE_STATIC, AW2725QF_ZONE_ALL, colors[0], 0, logo_mode.brightness, AW2725QF_SPEED_MEDIUM);
    }
    else
    {
        for(unsigned int led_idx = 0; led_idx < leds.size(); led_idx++)
        {
            DeviceUpdateSingleLED(led_idx);
        }
    }
}

void RGBController_AlienwareAW2725QF::DeviceUpdateZoneLEDs(int zone)
{
    DeviceUpdateSingleLED(zone);
}

void RGBController_AlienwareAW2725QF::DeviceUpdateSingleLED(int led)
{
    if(led < 0 || led >= (int)leds.size() || (modes[active_mode].flags & MODE_FLAG_REQUIRES_ENTIRE_DEVICE))
    {
        return;
    }

    const mode& selected_mode = GetEffectiveMode(led);
    if(selected_mode.value == AW2725QF_MODE_DIRECT)
    {
        controller->SetMode(AW2725QF_MODE_STATIC, leds[led].value, colors[led], 0, selected_mode.brightness, AW2725QF_SPEED_MEDIUM);
    }
}

void RGBController_AlienwareAW2725QF::DeviceUpdateMode()
{
    bool zone_override = false;
    for(unsigned int zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        if(zones[zone_idx].active_mode >= 0)
        {
            zone_override = true;
        }
    }

    if(!zone_override || (modes[active_mode].flags & MODE_FLAG_REQUIRES_ENTIRE_DEVICE))
    {
        if(modes[active_mode].value == AW2725QF_MODE_DIRECT)
        {
            DeviceUpdateLEDs();
        }
        else
        {
            SendMode(AW2725QF_ZONE_ALL, modes[active_mode]);
        }
    }
    else
    {
        for(unsigned int zone_idx = 0; zone_idx < zones.size(); zone_idx++)
        {
            DeviceUpdateZoneMode(zone_idx);
        }
    }
}

void RGBController_AlienwareAW2725QF::DeviceUpdateZoneMode(int zone)
{
    if(zone < 0 || zone >= (int)zones.size() || (modes[active_mode].flags & MODE_FLAG_REQUIRES_ENTIRE_DEVICE))
    {
        return;
    }

    const mode& selected_mode = GetEffectiveMode(zone);
    if(selected_mode.value == AW2725QF_MODE_DIRECT)
    {
        DeviceUpdateSingleLED(zone);
    }
    else
    {
        SendMode(leds[zone].value, selected_mode);
    }
}

void RGBController_AlienwareAW2725QF::SendMode(unsigned char led_mask, const mode& selected_mode)
{
    RGBColor color1 = selected_mode.colors.empty() ? 0 : selected_mode.colors[0];
    RGBColor color2 = selected_mode.colors.size() < 2 ? 0 : selected_mode.colors[1];

    controller->SetMode(selected_mode.value, led_mask, color1, color2, selected_mode.brightness, selected_mode.speed);
}
