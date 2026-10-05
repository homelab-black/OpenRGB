/*---------------------------------------------------------*\
| RGBController_Sinowealth.cpp                              |
|                                                           |
|   RGBController for Sinowealth mice, including Glorious   |
|                                                           |
|   Niels Westphal (crashniels)                 20 May 2020 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_Sinowealth.h"

/*---------------------------------------------------------*\
| Where each mode keeps its brightness/speed byte and its    |
| colors in the configuration packet. Static packs only      |
| brightness, the rest pack brightness in the high nibble    |
| and speed in the low one.                                  |
\*---------------------------------------------------------*/
struct sinowealth_mode_layout
{
    unsigned char   mode_value;
    unsigned int    param_offset;
    unsigned int    color_offset;
    unsigned int    color_count;
};

static const sinowealth_mode_layout sinowealth_mode_layouts[] =
{
    { GLORIOUS_MODE_RAINBOW,            0x36, 0x00, 0 },
    { GLORIOUS_MODE_STATIC,             0x38, 0x39, 1 },
    { GLORIOUS_MODE_SPECTRUM_BREATING,  0x3C, 0x3E, 7 },
    { GLORIOUS_MODE_TAIL,               0x53, 0x00, 0 },
    { GLORIOUS_MODE_SPECTRUM_CYCLE,     0x54, 0x00, 0 },
    { GLORIOUS_MODE_RAVE,               0x74, 0x75, 2 },
    { GLORIOUS_MODE_WAVE,               0x7C, 0x00, 0 },
    { GLORIOUS_MODE_BREATHING,          0x7D, 0x7E, 1 }
};

#define SINOWEALTH_MODE_OFFSET  0x35

/**------------------------------------------------------------------*\
    @name Sinowealth Mice
    @category Mouse
    @type USB
    @save :robot:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectSinowealthMouse,DetectTrustGXT970
    @comment
\*-------------------------------------------------------------------*/

RGBController_Sinowealth::RGBController_Sinowealth(SinowealthController* controller_ptr)
{
    controller                      = controller_ptr;

    name                            = controller->GetName();
    type                            = DEVICE_TYPE_MOUSE;
    description                     = "Sinowealth Device";
    location                        = controller->GetLocation();
    serial                          = controller->GetSerialString();
    version                         = controller->GetFirmwareVersion();

    mode Static;
    Static.name                     = "Custom";
    Static.flags                    = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Static.brightness_min           = GLORIOUS_BRIGHTNESS_LOW;
    Static.brightness               = GLORIOUS_BRIGHTNESS_NORMAL;
    Static.brightness_max           = GLORIOUS_BRIGHTNESS_HIGH;
    Static.color_mode               = MODE_COLORS_PER_LED;
    Static.value                    = GLORIOUS_MODE_STATIC;
    modes.push_back(Static);

    mode Off;
    Off.name                        = "Off";
    Off.flags                       = MODE_FLAG_AUTOMATIC_SAVE;
    Off.color_mode                  = MODE_COLORS_NONE;
    Off.value                       = GLORIOUS_MODE_OFF;
    modes.push_back(Off);

    mode Rainbow;
    Rainbow.name                    = "Rainbow";
    Rainbow.flags                   = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_UD | MODE_FLAG_AUTOMATIC_SAVE;
    Rainbow.speed_min               = GLORIOUS_SPEED_SLOW;
    Rainbow.speed                   = GLORIOUS_SPEED_NORMAL;
    Rainbow.speed_max               = GLORIOUS_SPEED_FAST;
    Rainbow.direction               = MODE_DIRECTION_UP;
    Rainbow.color_mode              = MODE_COLORS_NONE;
    Rainbow.value                   = GLORIOUS_MODE_RAINBOW;
    modes.push_back(Rainbow);

    mode SpectrumBreathing;
    SpectrumBreathing.name          = "Seemless Breathing";
    SpectrumBreathing.flags         = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    SpectrumBreathing.speed_min     = GLORIOUS_SPEED_SLOW;
    SpectrumBreathing.speed         = GLORIOUS_SPEED_NORMAL;
    SpectrumBreathing.speed_max     = GLORIOUS_SPEED_FAST;
    SpectrumBreathing.colors_min    = 7;
    SpectrumBreathing.colors_max    = 7;
    SpectrumBreathing.color_mode    = MODE_COLORS_MODE_SPECIFIC;
    SpectrumBreathing.value         = GLORIOUS_MODE_SPECTRUM_BREATING;
    SpectrumBreathing.colors.resize(7);
    modes.push_back(SpectrumBreathing);

    mode Chase;
    Chase.name                      = "Tail";
    Chase.flags                     = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Chase.speed_min                 = GLORIOUS_SPEED_SLOW;
    Chase.speed                     = GLORIOUS_SPEED_NORMAL;
    Chase.speed_max                 = GLORIOUS_SPEED_FAST;
    Chase.brightness_min            = GLORIOUS_BRIGHTNESS_LOW;
    Chase.brightness                = GLORIOUS_BRIGHTNESS_NORMAL;
    Chase.brightness_max            = GLORIOUS_BRIGHTNESS_HIGH;
    Chase.color_mode                = MODE_COLORS_NONE;
    Chase.value                     = GLORIOUS_MODE_TAIL;
    modes.push_back(Chase);

    mode SpectrumCycle;
    SpectrumCycle.name              = "Spectrum Cycle";
    SpectrumCycle.flags             = MODE_FLAG_HAS_SPEED | MODE_FLAG_AUTOMATIC_SAVE;
    SpectrumCycle.speed_min         = GLORIOUS_SPEED_SLOW;
    SpectrumCycle.speed             = GLORIOUS_SPEED_NORMAL;
    SpectrumCycle.speed_max         = GLORIOUS_SPEED_FAST;
    SpectrumCycle.color_mode        = MODE_COLORS_NONE;
    SpectrumCycle.value             = GLORIOUS_MODE_SPECTRUM_CYCLE;
    modes.push_back(SpectrumCycle);

    mode Flashing;
    Flashing.name                   = "Rave";
    Flashing.flags                  = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Flashing.speed_min              = GLORIOUS_SPEED_SLOW;
    Flashing.speed                  = GLORIOUS_SPEED_NORMAL;
    Flashing.speed_max              = GLORIOUS_SPEED_FAST;
    Flashing.brightness_min         = GLORIOUS_BRIGHTNESS_LOW;
    Flashing.brightness             = GLORIOUS_BRIGHTNESS_NORMAL;
    Flashing.brightness_max         = GLORIOUS_BRIGHTNESS_HIGH;
    Flashing.colors_min             = 2;
    Flashing.colors_max             = 2;
    Flashing.color_mode             = MODE_COLORS_MODE_SPECIFIC;
    Flashing.value                  = GLORIOUS_MODE_RAVE;
    Flashing.colors.resize(2);
    modes.push_back(Flashing);

    mode Epilepsy;
    Epilepsy.name                   = "Epilepsy";
    Epilepsy.flags                  = MODE_FLAG_AUTOMATIC_SAVE;
    Epilepsy.color_mode             = MODE_COLORS_NONE;
    Epilepsy.value                  = GLORIOUS_MODE_EPILEPSY;
    modes.push_back(Epilepsy);

    mode Wave;
    Wave.name                       = "Wave";
    Wave.flags                      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Wave.speed_min                  = GLORIOUS_SPEED_SLOW;
    Wave.speed                      = GLORIOUS_SPEED_NORMAL;
    Wave.speed_max                  = GLORIOUS_SPEED_FAST;
    Wave.brightness_min             = GLORIOUS_BRIGHTNESS_LOW;
    Wave.brightness                 = GLORIOUS_BRIGHTNESS_NORMAL;
    Wave.brightness_max             = GLORIOUS_BRIGHTNESS_HIGH;
    Wave.color_mode                 = MODE_COLORS_NONE;
    Wave.value                      = GLORIOUS_MODE_WAVE;
    modes.push_back(Wave);

    mode Breathing;
    Breathing.name                  = "Breathing";
    Breathing.flags                 = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
    Breathing.speed_min             = GLORIOUS_SPEED_SLOW;
    Breathing.speed                 = GLORIOUS_SPEED_NORMAL;
    Breathing.speed_max             = GLORIOUS_SPEED_FAST;
    Breathing.colors_min            = 1;
    Breathing.colors_max            = 1;
    Breathing.color_mode            = MODE_COLORS_MODE_SPECIFIC;
    Breathing.value                 = GLORIOUS_MODE_BREATHING;
    Breathing.colors.resize(1);
    modes.push_back(Breathing);

    SetupZones();

    /*-----------------------------------------------------*\
    | Start out matching the device. Without this the        |
    | controller reports mode 0 with black colors, and the   |
    | first thing that applies a profile turns the device    |
    | off.                                                   |
    \*-----------------------------------------------------*/
    bool state_known = ReadDeviceState();

    /*-----------------------------------------------------*\
    | Some variants stop lighting up once their config has   |
    | been read and need it written back. Only do that when  |
    | the read worked, writing a state we never read would   |
    | put the device out rather than bring it back.          |
    \*-----------------------------------------------------*/
    if(state_known && controller->RequiresStartupRefresh())
    {
        DeviceUpdateMode();
    }
}

bool RGBController_Sinowealth::ReadDeviceState()
{
    if(controller->GetProfile() < SINOWEALTH_CONFIG_SIZE_MIN)
    {
        return(false);
    }

    /*-----------------------------------------------------*\
    | Select the mode the device is currently in. A mode we  |
    | do not implement leaves the default selection alone.   |
    \*-----------------------------------------------------*/
    unsigned char mode_value = controller->GetConfigByte(SINOWEALTH_MODE_OFFSET);

    for(std::size_t mode_idx = 0; mode_idx < modes.size(); mode_idx++)
    {
        if(modes[mode_idx].value == mode_value)
        {
            active_mode = (int)mode_idx;
            break;
        }
    }

    /*-----------------------------------------------------*\
    | Restore each mode's stored parameters and colors       |
    \*-----------------------------------------------------*/
    for(const sinowealth_mode_layout& layout : sinowealth_mode_layouts)
    {
        for(mode& current : modes)
        {
            if(current.value != layout.mode_value)
            {
                continue;
            }

            unsigned char param = controller->GetConfigByte(layout.param_offset);

            if(current.flags & MODE_FLAG_HAS_BRIGHTNESS)
            {
                unsigned int brightness = (param >> 4) & 0xF;

                if(brightness >= current.brightness_min && brightness <= current.brightness_max)
                {
                    current.brightness = brightness;
                }
            }

            if(current.flags & MODE_FLAG_HAS_SPEED)
            {
                unsigned int speed = param & 0xF;

                if(speed >= current.speed_min && speed <= current.speed_max)
                {
                    current.speed = speed;
                }
            }

            for(unsigned int color_idx = 0; color_idx < layout.color_count && color_idx < current.colors.size(); color_idx++)
            {
                current.colors[color_idx] = controller->GetConfigColor(layout.color_offset + (color_idx * 3));
            }

            break;
        }
    }

    /*-----------------------------------------------------*\
    | The per LED colors live in the static mode registers,  |
    | plus the DPI stage block on variants that have one     |
    \*-----------------------------------------------------*/
    if(colors.size() > SINOWEALTH_LED_EFFECT)
    {
        colors[SINOWEALTH_LED_EFFECT] = controller->GetConfigColor(0x39);
    }

    if(colors.size() > SINOWEALTH_LED_DPI)
    {
        colors[SINOWEALTH_LED_DPI] = controller->GetConfigColor(SINOWEALTH_DPI_COLOR_OFFSET);
    }

    return(true);
}

RGBController_Sinowealth::~RGBController_Sinowealth()
{
    Shutdown();

    delete controller;
}

void RGBController_Sinowealth::SetupZones()
{
    /*---------------------------------------------------------*\
    | One zone per LED. The effect the device runs only lights   |
    | the first one, so anything past that gets a zone of its    |
    | own with a mode list to match.                             |
    \*---------------------------------------------------------*/
    for(unsigned int led_idx = 0; led_idx < controller->GetLEDCount(); led_idx++)
    {
        /*-----------------------------------------------------*\
        | Single LED devices keep the zone name they have had     |
        | all along, saved profiles are matched on it             |
        \*-----------------------------------------------------*/
        zone new_zone;
        new_zone.name       = (controller->GetLEDCount() > 1) ? controller->GetLEDName(led_idx) : "Mouse";
        new_zone.type       = ZONE_TYPE_SINGLE;
        new_zone.leds_min   = 1;
        new_zone.leds_max   = 1;
        new_zone.leds_count = 1;

        /*-----------------------------------------------------*\
        | The DPI stage colors are untouched by every effect, so |
        | this zone can hold a color while the device runs one.  |
        |                                                        |
        | It selects its own mode rather than following the      |
        | device. A zone left following contributes no color     |
        | mode of its own, so while the device runs an effect    |
        | the color picker has nothing to apply and silently     |
        | drops what the user chose.                             |
        \*-----------------------------------------------------*/
        if(led_idx == SINOWEALTH_LED_DPI)
        {
            mode Static;
            Static.name         = "Static";
            Static.flags        = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
            Static.color_mode   = MODE_COLORS_PER_LED;
            Static.value        = GLORIOUS_MODE_STATIC;
            new_zone.modes.push_back(Static);

            new_zone.active_mode = 0;
        }

        /*-----------------------------------------------------*\
        | The effects only ever light this zone, so offer them   |
        | here as well as on the device. Picking one on the zone |
        | is the same as picking it on the device, there is only |
        | one effect engine, but this is where it is visible.    |
        \*-----------------------------------------------------*/
        if((led_idx == SINOWEALTH_LED_EFFECT) && (controller->GetLEDCount() > 1))
        {
            new_zone.modes       = modes;
            new_zone.active_mode = -1;
        }

        zones.push_back(new_zone);

        led* new_led = new led();
        new_led->name = controller->GetLEDName(led_idx);
        leds.push_back(*new_led);
    }

    SetupColors();
}

/*---------------------------------------------------------*\
| True for the zone whose color comes from the DPI stage     |
| block rather than from the effect                          |
\*---------------------------------------------------------*/
bool RGBController_Sinowealth::IsDPIZone(int zone)
{
    return((zone >= 0) && ((unsigned int)zone == SINOWEALTH_LED_DPI) && (zones.size() > SINOWEALTH_LED_DPI));
}

void RGBController_Sinowealth::DeviceUpdateLEDs()
{
    DeviceUpdateMode();
}

void RGBController_Sinowealth::DeviceUpdateZoneLEDs(int zone)
{
    if(IsDPIZone(zone))
    {
        controller->SetDPIColor(colors[SINOWEALTH_LED_DPI]);
        return;
    }

    DeviceUpdateMode();
}

void RGBController_Sinowealth::DeviceUpdateSingleLED(int led)
{
    DeviceUpdateZoneLEDs(led);
}

void RGBController_Sinowealth::DeviceUpdateZoneMode(int zone)
{
    if(IsDPIZone(zone))
    {
        controller->SetDPIColor(colors[SINOWEALTH_LED_DPI]);
        return;
    }

    /*-----------------------------------------------------*\
    | The effect zone drives the one effect engine the       |
    | device has, so applying its mode is applying the       |
    | device's. A zone set to follow just uses the device's.  |
    |                                                        |
    | Mirror the selection onto the device mode rather than   |
    | only applying it. The two would otherwise disagree, and |
    | the next device level update, setting a color among     |
    | them, would quietly revert the effect.                  |
    \*-----------------------------------------------------*/
    if((zone >= 0) && (zone < (int)zones.size()) && (zones[zone].active_mode >= 0)
    && (zones[zone].active_mode < (int)zones[zone].modes.size())
    && (zones[zone].active_mode < (int)modes.size()))
    {
        active_mode        = zones[zone].active_mode;
        modes[active_mode] = zones[zone].modes[zones[zone].active_mode];

        ApplyMode(modes[active_mode]);
        return;
    }

    DeviceUpdateMode();
}

void RGBController_Sinowealth::DeviceUpdateMode()
{
    ApplyMode(modes[active_mode]);
}

void RGBController_Sinowealth::ApplyMode(mode& current)
{
    /*-----------------------------------------------------*\
    | Hand the logo color over before the write so it rides   |
    | along in the same packet as the effect                  |
    \*-----------------------------------------------------*/
    if(colors.size() > SINOWEALTH_LED_DPI)
    {
        controller->StageDPIColor(colors[SINOWEALTH_LED_DPI]);
    }

    unsigned int direction = 0;
    unsigned int speed = GLORIOUS_SPEED_FAST;
    unsigned int brightness = GLORIOUS_BRIGHTNESS_HIGH;

    if (current.flags & MODE_FLAG_HAS_SPEED)
    {
        speed = current.speed;
    }

    if (current.flags & MODE_FLAG_HAS_BRIGHTNESS)
    {
        brightness = current.brightness;
    }

    if ((current.flags & MODE_FLAG_HAS_DIRECTION_LR)  ||
        (current.flags & MODE_FLAG_HAS_DIRECTION_UD)  ||
        (current.flags & MODE_FLAG_HAS_DIRECTION_HV))
    {
        if (current.direction == MODE_DIRECTION_UP)
        {
            direction = GLORIOUS_DIRECTION_UP;
        }
        else
        {
            direction = GLORIOUS_DIRECTION_DOWN;
        }
    }

    if (current.color_mode == MODE_COLORS_NONE)
    {
        controller->SetMode(current.value, speed, brightness, direction, 0);
    }
    else if (current.color_mode == MODE_COLORS_PER_LED)
    {
        controller->SetMode(current.value, speed, brightness, direction, &colors[0]);
    }
    else
    {
        controller->SetMode(current.value, speed, brightness, direction, &current.colors[0]);
    }
}

