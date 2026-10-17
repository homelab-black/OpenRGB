/*---------------------------------------------------------*\
| RGBController_LianLiUniHubSLInfinity.cpp                  |
|                                                           |
|   RGBController for Lian Li SL Infinity Uni Hub           |
|                                                           |
|   Pau Verdeguer                               13 May 2026 |
|   Simon McKenna                               21 Oct 2023 |
|   Will Kennedy                                17 Jan 2023 |
|   Oliver P                                    26 Apr 2022 |
|   Credit to Luca Lovisa for original work.                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <string>
#include "RGBController_LianLiUniHubSLInfinity.h"

/**------------------------------------------------------------------*\
    @name Lian Li Uni Hub SL Infinity
    @type USB
    @save :x:
    @direct :rotating_light:
    @effects :white_check_mark:
    @detectors DetectLianLiUniHubSLInfinity
    @comment The hub has no direct mode. Custom sets every LED
        with the static effect, which takes about 25 ms per ring
        that changes.
\*-------------------------------------------------------------------*/

/*---------------------------------------------------------*\
| Default effect colors, the same ones L-Connect uses.      |
\*---------------------------------------------------------*/
static const RGBColor default_colors[4] =
{
    ToRGBColor(0xFF, 0x00, 0x00),
    ToRGBColor(0x00, 0x00, 0xFF),
    ToRGBColor(0x00, 0xFF, 0x55),
    ToRGBColor(0xFF, 0x69, 0x00),
};

static void SetModeColors(mode& m, unsigned int count)
{
    m.colors_min = count;
    m.colors_max = count;
    m.colors.assign(default_colors, default_colors + count);
}

static bool IsInnerChannel(int zone_idx)
{
    return((zone_idx % 2) == 0);
}

/*---------------------------------------------------------*\
| Effects that L-Connect only offers on the inner ring.     |
\*---------------------------------------------------------*/
static bool IsInnerOnlyEffect(unsigned char value)
{
    switch(value)
    {
        case UNIHUB_SLINF_LED_MODE_TAICHI:
        case UNIHUB_SLINF_LED_MODE_DOUBLE_METEOR:
        case UNIHUB_SLINF_LED_MODE_METEOR_CONTEST:
        case UNIHUB_SLINF_LED_MODE_METEOR_MIX:
        case UNIHUB_SLINF_LED_MODE_RETURN_ARC:
        case UNIHUB_SLINF_LED_MODE_DOUBLE_ARC:
        case UNIHUB_SLINF_LED_MODE_HEART_BEAT_RUNWAY:
            return(true);
        default:
            return(false);
    }
}

/*---------------------------------------------------------*\
| Effects that L-Connect only offers on the outer ring.     |
\*---------------------------------------------------------*/
static bool IsOuterOnlyEffect(unsigned char value)
{
    switch(value)
    {
        case UNIHUB_SLINF_LED_MODE_COLORFUL_METEOR:
        case UNIHUB_SLINF_LED_MODE_REFLECT:
            return(true);
        default:
            return(false);
    }
}

static bool IsSimultaneousEffect(unsigned char value)
{
    switch(value)
    {
        case UNIHUB_SLINF_LED_MODE_RAINBOW_SIM:
        case UNIHUB_SLINF_LED_MODE_RAINBOW_MORPH_SIM:
        case UNIHUB_SLINF_LED_MODE_WARNING_SIM:
        case UNIHUB_SLINF_LED_MODE_VOICE_SIM:
        case UNIHUB_SLINF_LED_MODE_MIXING_SIM:
        case UNIHUB_SLINF_LED_MODE_STACK_SIM:
        case UNIHUB_SLINF_LED_MODE_TIDE_SIM:
        case UNIHUB_SLINF_LED_MODE_SCAN_SIM:
        case UNIHUB_SLINF_LED_MODE_DOOR_SIM:
        case UNIHUB_SLINF_LED_MODE_ELECTRIC_CURRENT_SIM:
        case UNIHUB_SLINF_LED_MODE_HEART_BEAT_SIM:
        case UNIHUB_SLINF_LED_MODE_HEART_BEAT_RUNWAY_SIM:
        case UNIHUB_SLINF_LED_MODE_MOP_UP_SIM:
            return(true);
        default:
            return(false);
    }
}

static bool IsMergedEffect(unsigned char value)
{
    switch(value)
    {
        case UNIHUB_SLINF_LED_MODE_RUNWAY_MERGED:
        case UNIHUB_SLINF_LED_MODE_MOP_UP_MERGED:
        case UNIHUB_SLINF_LED_MODE_MIXING_MERGED:
        case UNIHUB_SLINF_LED_MODE_STACK_MERGED:
        case UNIHUB_SLINF_LED_MODE_TIDE_MERGED:
        case UNIHUB_SLINF_LED_MODE_SCAN_MERGED:
        case UNIHUB_SLINF_LED_MODE_DOOR_MERGED:
        case UNIHUB_SLINF_LED_MODE_HEART_BEAT_RUNWAY_MERGED:
        case UNIHUB_SLINF_LED_MODE_ELECTRIC_CURRENT_MERGED:
            return(true);
        default:
            return(false);
    }
}

RGBController_LianLiUniHubSLInfinity::RGBController_LianLiUniHubSLInfinity(LianLiUniHubSLInfinityController* controller_ptr)
{
    controller          = controller_ptr;
    needs_full_update   = true;

    name                = controller->GetName();
    vendor              = "Lian Li";
    type                = DEVICE_TYPE_COOLER;
    description         = "Lian Li Uni Hub - SL Infinity";
    version             = controller->GetFirmwareVersionString();
    location            = controller->GetDeviceLocation();
    serial              = controller->GetSerialString();

    /*-----------------------------------------------------*\
    | With the pacing an update can take a few hundred ms,  |
    | so the update request is cleared before the update    |
    | runs and changes made meanwhile are not lost.         |
    \*-----------------------------------------------------*/
    flags              |= CONTROLLER_FLAG_RESET_BEFORE_UPDATE;

    mode Custom;
    Custom.name                 = "Custom";
    Custom.value                = UNIHUB_SLINF_LED_MODE_STATIC_COLOR;
    Custom.flags                = MODE_FLAG_HAS_PER_LED_COLOR;
    Custom.color_mode           = MODE_COLORS_PER_LED;
    modes.push_back(Custom);

    mode Breathing;
    Breathing.name              = "Breathing";
    Breathing.value             = UNIHUB_SLINF_LED_MODE_BREATHING;
    Breathing.flags             = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_PER_LED_COLOR;
    Breathing.brightness_min    = 0;
    Breathing.brightness_max    = 4;
    Breathing.brightness        = 4;
    Breathing.speed_min         = 0;
    Breathing.speed_max         = 4;
    Breathing.speed             = 2;
    Breathing.color_mode        = MODE_COLORS_PER_LED;
    modes.push_back(Breathing);

    /*-----------------------------------------------------*\
    | The rest of the modes follow L-Connect 3: per-ring    |
    | effects, Simultaneous effects (both rings of a port)  |
    | and Merged effects (all ports). Color counts, speed   |
    | and direction match what L-Connect offers.            |
    \*-----------------------------------------------------*/
    mode NewMode;
    NewMode.brightness_min      = 0;
    NewMode.brightness_max      = 4;
    NewMode.brightness          = 4;
    NewMode.speed_min           = 0;
    NewMode.speed_max           = 4;
    NewMode.speed               = 3;
    NewMode.direction           = MODE_DIRECTION_RIGHT;

    NewMode.flags               = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR;
    NewMode.color_mode          = MODE_COLORS_NONE;
    SetModeColors(NewMode, 0);
    NewMode.name = "Rainbow";                          NewMode.value = UNIHUB_SLINF_LED_MODE_RAINBOW;                  modes.push_back(NewMode);
    NewMode.name = "Simultaneous Rainbow";             NewMode.value = UNIHUB_SLINF_LED_MODE_RAINBOW_SIM;              modes.push_back(NewMode);
    NewMode.name = "Meteor Rainbow";                   NewMode.value = UNIHUB_SLINF_LED_MODE_METEOR_RAINBOW;           modes.push_back(NewMode);
    NewMode.name = "Colorful Meteor (Outer Only)";     NewMode.value = UNIHUB_SLINF_LED_MODE_COLORFUL_METEOR;          modes.push_back(NewMode);

    NewMode.flags               = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;
    NewMode.name = "Rainbow Morph";                    NewMode.value = UNIHUB_SLINF_LED_MODE_RAINBOW_MORPH;            modes.push_back(NewMode);
    NewMode.name = "Simultaneous Rainbow Morph";       NewMode.value = UNIHUB_SLINF_LED_MODE_RAINBOW_MORPH_SIM;        modes.push_back(NewMode);
    NewMode.name = "Breathing Rainbow";                NewMode.value = UNIHUB_SLINF_LED_MODE_BREATHING_RAINBOW;        modes.push_back(NewMode);

    NewMode.color_mode          = MODE_COLORS_MODE_SPECIFIC;
    NewMode.flags               = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    SetModeColors(NewMode, 4);
    NewMode.name = "Mop Up";                           NewMode.value = UNIHUB_SLINF_LED_MODE_MOP_UP;                   modes.push_back(NewMode);
    NewMode.name = "Simultaneous Mop Up";              NewMode.value = UNIHUB_SLINF_LED_MODE_MOP_UP_SIM;               modes.push_back(NewMode);
    NewMode.name = "Warning";                          NewMode.value = UNIHUB_SLINF_LED_MODE_WARNING;                  modes.push_back(NewMode);
    NewMode.name = "Simultaneous Warning";             NewMode.value = UNIHUB_SLINF_LED_MODE_WARNING_SIM;              modes.push_back(NewMode);
    NewMode.name = "Tide";                             NewMode.value = UNIHUB_SLINF_LED_MODE_TIDE;                     modes.push_back(NewMode);
    NewMode.name = "Simultaneous Tide";                NewMode.value = UNIHUB_SLINF_LED_MODE_TIDE_SIM;                 modes.push_back(NewMode);
    NewMode.name = "Door";                             NewMode.value = UNIHUB_SLINF_LED_MODE_DOOR;                     modes.push_back(NewMode);
    NewMode.name = "Simultaneous Door";                NewMode.value = UNIHUB_SLINF_LED_MODE_DOOR_SIM;                 modes.push_back(NewMode);
    NewMode.name = "Simultaneous Electric Current";    NewMode.value = UNIHUB_SLINF_LED_MODE_ELECTRIC_CURRENT_SIM;     modes.push_back(NewMode);
    NewMode.name = "Double Meteor (Inner Only)";       NewMode.value = UNIHUB_SLINF_LED_MODE_DOUBLE_METEOR;            modes.push_back(NewMode);

    NewMode.flags               = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    NewMode.name = "Color Cycle";                      NewMode.value = UNIHUB_SLINF_LED_MODE_COLOR_CYCLE;              modes.push_back(NewMode);
    NewMode.name = "Meteor";                           NewMode.value = UNIHUB_SLINF_LED_MODE_METEOR;                   modes.push_back(NewMode);
    NewMode.name = "Voice";                            NewMode.value = UNIHUB_SLINF_LED_MODE_VOICE;                    modes.push_back(NewMode);
    NewMode.name = "Disco";                            NewMode.value = UNIHUB_SLINF_LED_MODE_DISCO;                    modes.push_back(NewMode);
    NewMode.name = "Return Arc (Inner Only)";          NewMode.value = UNIHUB_SLINF_LED_MODE_RETURN_ARC;               modes.push_back(NewMode);
    NewMode.name = "Double Arc (Inner Only)";          NewMode.value = UNIHUB_SLINF_LED_MODE_DOUBLE_ARC;               modes.push_back(NewMode);

    SetModeColors(NewMode, 2);
    NewMode.name = "Simultaneous Voice";               NewMode.value = UNIHUB_SLINF_LED_MODE_VOICE_SIM;                modes.push_back(NewMode);
    NewMode.name = "Lottery";                          NewMode.value = UNIHUB_SLINF_LED_MODE_LOTTERY;                  modes.push_back(NewMode);
    NewMode.name = "Stack";                            NewMode.value = UNIHUB_SLINF_LED_MODE_STACK;                    modes.push_back(NewMode);
    NewMode.name = "Simultaneous Stack";               NewMode.value = UNIHUB_SLINF_LED_MODE_STACK_SIM;                modes.push_back(NewMode);
    NewMode.name = "Taichi (Inner Only)";              NewMode.value = UNIHUB_SLINF_LED_MODE_TAICHI;                   modes.push_back(NewMode);
    NewMode.name = "Meteor Contest (Inner Only)";      NewMode.value = UNIHUB_SLINF_LED_MODE_METEOR_CONTEST;           modes.push_back(NewMode);

    NewMode.flags               = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    NewMode.name = "Runway";                           NewMode.value = UNIHUB_SLINF_LED_MODE_RUNWAY;                   modes.push_back(NewMode);
    NewMode.name = "Mixing";                           NewMode.value = UNIHUB_SLINF_LED_MODE_MIXING;                   modes.push_back(NewMode);
    NewMode.name = "Simultaneous Mixing";              NewMode.value = UNIHUB_SLINF_LED_MODE_MIXING_SIM;               modes.push_back(NewMode);
    NewMode.name = "Simultaneous Scan";                NewMode.value = UNIHUB_SLINF_LED_MODE_SCAN_SIM;                 modes.push_back(NewMode);
    NewMode.name = "Meteor Mix (Inner Only)";          NewMode.value = UNIHUB_SLINF_LED_MODE_METEOR_MIX;               modes.push_back(NewMode);

    SetModeColors(NewMode, 1);
    NewMode.name = "Heart Beat";                       NewMode.value = UNIHUB_SLINF_LED_MODE_HEART_BEAT;               modes.push_back(NewMode);
    NewMode.name = "Simultaneous Heart Beat";          NewMode.value = UNIHUB_SLINF_LED_MODE_HEART_BEAT_SIM;           modes.push_back(NewMode);
    NewMode.name = "Scan";                             NewMode.value = UNIHUB_SLINF_LED_MODE_SCAN;                     modes.push_back(NewMode);
    NewMode.name = "Reflect (Outer Only)";             NewMode.value = UNIHUB_SLINF_LED_MODE_REFLECT;                  modes.push_back(NewMode);

    NewMode.flags               = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    NewMode.name = "Simultaneous Heart Beat Runway";   NewMode.value = UNIHUB_SLINF_LED_MODE_HEART_BEAT_RUNWAY_SIM;    modes.push_back(NewMode);
    NewMode.name = "Heart Beat Runway (Inner Only)";   NewMode.value = UNIHUB_SLINF_LED_MODE_HEART_BEAT_RUNWAY;        modes.push_back(NewMode);

    /*-----------------------------------------------------*\
    | Merged effects take over all ports, so they can only  |
    | be set for the whole device.                          |
    \*-----------------------------------------------------*/
    NewMode.flags               = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_REQUIRES_ENTIRE_DEVICE;
    SetModeColors(NewMode, 2);
    NewMode.name = "Merged Runway";                    NewMode.value = UNIHUB_SLINF_LED_MODE_RUNWAY_MERGED;            modes.push_back(NewMode);
    NewMode.name = "Merged Mixing";                    NewMode.value = UNIHUB_SLINF_LED_MODE_MIXING_MERGED;            modes.push_back(NewMode);
    NewMode.name = "Merged Scan";                      NewMode.value = UNIHUB_SLINF_LED_MODE_SCAN_MERGED;              modes.push_back(NewMode);

    NewMode.flags               = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_REQUIRES_ENTIRE_DEVICE;
    NewMode.name = "Merged Stack";                     NewMode.value = UNIHUB_SLINF_LED_MODE_STACK_MERGED;             modes.push_back(NewMode);

    NewMode.flags               = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_REQUIRES_ENTIRE_DEVICE;
    SetModeColors(NewMode, 4);
    NewMode.name = "Merged Mop Up";                    NewMode.value = UNIHUB_SLINF_LED_MODE_MOP_UP_MERGED;            modes.push_back(NewMode);
    NewMode.name = "Merged Tide";                      NewMode.value = UNIHUB_SLINF_LED_MODE_TIDE_MERGED;              modes.push_back(NewMode);
    NewMode.name = "Merged Door";                      NewMode.value = UNIHUB_SLINF_LED_MODE_DOOR_MERGED;              modes.push_back(NewMode);
    NewMode.name = "Merged Electric Current";          NewMode.value = UNIHUB_SLINF_LED_MODE_ELECTRIC_CURRENT_MERGED;  modes.push_back(NewMode);

    NewMode.flags               = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_REQUIRES_ENTIRE_DEVICE;
    SetModeColors(NewMode, 1);
    NewMode.name = "Merged Heart Beat Runway";         NewMode.value = UNIHUB_SLINF_LED_MODE_HEART_BEAT_RUNWAY_MERGED; modes.push_back(NewMode);

    SetupZones();
}

RGBController_LianLiUniHubSLInfinity::~RGBController_LianLiUniHubSLInfinity()
{
    Shutdown();

    delete controller;
}

void RGBController_LianLiUniHubSLInfinity::SetupZones()
{
    bool first_run = zones.empty();

    static const char* default_zone_names[UNIHUB_SLINF_CHANNEL_COUNT] =
    {
        "Port 1 Inner Ring",
        "Port 1 Outer Ring",
        "Port 2 Inner Ring",
        "Port 2 Outer Ring",
        "Port 3 Inner Ring",
        "Port 3 Outer Ring",
        "Port 4 Inner Ring",
        "Port 4 Outer Ring",
    };

    leds.clear();
    colors.clear();
    zones.resize(UNIHUB_SLINF_CHANNEL_COUNT);

    for(unsigned int channel_idx = 0; channel_idx < zones.size(); channel_idx++)
    {
        unsigned int leds_per_fan = IsInnerChannel((int)channel_idx) ? UNIHUB_SLINF_INNER_LEDS_PER_FAN
                                                                     : UNIHUB_SLINF_OUTER_LEDS_PER_FAN;

        zones[channel_idx].leds_min = 0;
        zones[channel_idx].leds_max = UNIHUB_SLINF_MAX_FAN_COUNT * leds_per_fan;

        if(first_run)
        {
            zones[channel_idx].flags = ZONE_FLAG_MANUALLY_CONFIGURABLE_SIZE
                                     | ZONE_FLAG_MANUALLY_CONFIGURABLE_NAME
                                     | ZONE_FLAG_MANUALLY_CONFIGURABLE_TYPE
                                     | ZONE_FLAG_MANUALLY_CONFIGURABLE_MATRIX_MAP
                                     | ZONE_FLAG_MANUALLY_CONFIGURABLE_SEGMENTS;

            /*---------------------------------------------*\
            | Every zone gets its own copy of the modes, so |
            | the rings can run different effects.          |
            \*---------------------------------------------*/
            zones[channel_idx].modes       = modes;
            zones[channel_idx].active_mode = -1;
        }

        if(!(zones[channel_idx].flags & ZONE_FLAG_MANUALLY_CONFIGURED_NAME))
        {
            zones[channel_idx].name = default_zone_names[channel_idx];
        }

        if(!(zones[channel_idx].flags & ZONE_FLAG_MANUALLY_CONFIGURED_SIZE))
        {
            zones[channel_idx].leds_count = 0;
        }

        if(zones[channel_idx].leds_count > zones[channel_idx].leds_max)
        {
            zones[channel_idx].leds_count = zones[channel_idx].leds_max;
        }

        if(!(zones[channel_idx].flags & ZONE_FLAG_MANUALLY_CONFIGURED_TYPE))
        {
            zones[channel_idx].type = ZONE_TYPE_LINEAR;
        }

        if(!(zones[channel_idx].flags & ZONE_FLAG_MANUALLY_CONFIGURED_MATRIX_MAP))
        {
            zones[channel_idx].matrix_map.width  = 0;
            zones[channel_idx].matrix_map.height = 0;
            zones[channel_idx].matrix_map.map.resize(0);
        }

        for(unsigned int led_ch_idx = 0; led_ch_idx < zones[channel_idx].leds_count; led_ch_idx++)
        {
            led new_led;

            new_led.name  = zones[channel_idx].name + ", LED " + std::to_string(led_ch_idx + 1);
            new_led.value = channel_idx;

            leds.push_back(new_led);
        }
    }

    SetupColors();
    SendFanQuantities();
}

/*---------------------------------------------------------*\
| The hub needs the number of fans on each port to lay out  |
| its effects. It is taken from the zone sizes, using the   |
| larger ring of the port. Ports with no LEDs configured    |
| are left as they are.                                     |
\*---------------------------------------------------------*/
void RGBController_LianLiUniHubSLInfinity::SendFanQuantities()
{
    for(unsigned int group = 0; group < UNIHUB_SLINF_GROUP_COUNT; group++)
    {
        unsigned int inner_leds = zones[group * 2].leds_count;
        unsigned int outer_leds = zones[group * 2 + 1].leds_count;
        unsigned int inner_fans = (inner_leds + UNIHUB_SLINF_INNER_LEDS_PER_FAN - 1) / UNIHUB_SLINF_INNER_LEDS_PER_FAN;
        unsigned int outer_fans = (outer_leds + UNIHUB_SLINF_OUTER_LEDS_PER_FAN - 1) / UNIHUB_SLINF_OUTER_LEDS_PER_FAN;
        unsigned int fans       = (inner_fans > outer_fans) ? inner_fans : outer_fans;

        if(fans > 0)
        {
            controller->SetFanQuantity((unsigned char)group, (unsigned char)fans);
        }
    }
}

void RGBController_LianLiUniHubSLInfinity::DeviceConfigureZone(int zone_idx)
{
    std::lock_guard<std::mutex> guard(update_mutex);

    if((std::size_t)zone_idx < zones.size())
    {
        SetupZones();

        /*-------------------------------------------------*\
        | A new fan count changes the layout on the hub, so |
        | everything is sent again on the next update.      |
        \*-------------------------------------------------*/
        controller->MarkAllChannelsStale();
        needs_full_update = true;
    }
}

const mode& RGBController_LianLiUniHubSLInfinity::GetEffectiveModeForZone(int zone_idx) const
{
    int zone_active = zones[zone_idx].active_mode;

    if(zone_active >= 0 && (std::size_t)zone_active < zones[zone_idx].modes.size())
    {
        return(zones[zone_idx].modes[zone_active]);
    }

    return(modes[active_mode]);
}

/*---------------------------------------------------------*\
| Mode the ring of this zone shows, or NULL if the zone has |
| no LEDs or the ring cannot show its mode (Merged effects, |
| and effects L-Connect does not offer on this ring).       |
\*---------------------------------------------------------*/
const mode* RGBController_LianLiUniHubSLInfinity::GetRingMode(unsigned int zone_idx)
{
    if(zones[zone_idx].leds_count == 0)
    {
        return(NULL);
    }

    const mode&   effective = GetEffectiveModeForZone(zone_idx);
    unsigned char value     = (unsigned char)effective.value;
    bool          is_inner  = IsInnerChannel((int)zone_idx);

    if(IsMergedEffect(value) || (IsInnerOnlyEffect(value) && !is_inner) || (IsOuterOnlyEffect(value) && is_inner))
    {
        return(NULL);
    }

    return(&effective);
}

/*---------------------------------------------------------*\
| Merged effects take over the whole hub, so a single zone  |
| with LEDs using one is enough.                            |
\*---------------------------------------------------------*/
const mode* RGBController_LianLiUniHubSLInfinity::GetMergedMode()
{
    for(unsigned int zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        if(zones[zone_idx].leds_count > 0 && IsMergedEffect((unsigned char)GetEffectiveModeForZone(zone_idx).value))
        {
            return(&GetEffectiveModeForZone(zone_idx));
        }
    }

    return(NULL);
}

/*---------------------------------------------------------*\
| Returns true if an animated effect was sent, which is     |
| then synced with a SetFrame.                              |
\*---------------------------------------------------------*/
bool RGBController_LianLiUniHubSLInfinity::UpdateRing(unsigned int zone_idx, const mode& ring_mode)
{
    unsigned char channel = (unsigned char)zone_idx;

    if(ring_mode.color_mode == MODE_COLORS_PER_LED)
    {
        if(ring_mode.value == UNIHUB_SLINF_LED_MODE_BREATHING)
        {
            return(controller->SetChannelBreathing(channel, zones[zone_idx].colors, zones[zone_idx].leds_count, ring_mode));
        }

        controller->SetChannelLEDs(channel, zones[zone_idx].colors, zones[zone_idx].leds_count);

        return(false);
    }

    if(IsSimultaneousEffect((unsigned char)ring_mode.value))
    {
        return(controller->SetChannelModeSimultaneous(channel, ring_mode));
    }

    return(controller->SetChannelMode(channel, ring_mode));
}

/*---------------------------------------------------------*\
| While the inner ring runs a Simultaneous effect the outer |
| ring follows it, whatever it gets sent, and it freezes    |
| once the inner ring stops. So the outer ring shows the    |
| Simultaneous effect of its inner ring if there is one,    |
| and its own mode otherwise. An inner ring with nothing to |
| show runs the Simultaneous effect of its outer ring.      |
|                                                           |
| NULL means the ring is off or follows the inner ring.     |
\*---------------------------------------------------------*/
void RGBController_LianLiUniHubSLInfinity::GetPortModes(unsigned int port, const mode** inner_mode,
                                                        const mode** outer_mode, bool* outer_follows)
{
    *inner_mode = GetRingMode(port * 2);
    *outer_mode = GetRingMode(port * 2 + 1);

    if(*outer_mode != NULL && IsSimultaneousEffect((unsigned char)(*outer_mode)->value))
    {
        if(*inner_mode == NULL)
        {
            *inner_mode = *outer_mode;
        }

        *outer_mode = NULL;
    }

    *outer_follows = (*inner_mode != NULL) && IsSimultaneousEffect((unsigned char)(*inner_mode)->value);

    if(*outer_follows)
    {
        *outer_mode = NULL;
    }
}

bool RGBController_LianLiUniHubSLInfinity::UpdatePort(unsigned int port)
{
    const mode* inner_mode;
    const mode* outer_mode;
    bool        outer_follows;
    bool        animated = false;

    GetPortModes(port, &inner_mode, &outer_mode, &outer_follows);

    if(inner_mode != NULL)
    {
        animated |= UpdateRing(port * 2, *inner_mode);
    }
    else
    {
        controller->SetChannelOff((unsigned char)(port * 2));
    }

    if(outer_mode != NULL)
    {
        animated |= UpdateRing(port * 2 + 1, *outer_mode);
    }
    else if(!outer_follows)
    {
        controller->SetChannelOff((unsigned char)(port * 2 + 1));
    }

    return(animated);
}

bool RGBController_LianLiUniHubSLInfinity::NeedsFullUpdate()
{
    return(needs_full_update || GetMergedMode() != NULL || controller->IsMerged());
}

void RGBController_LianLiUniHubSLInfinity::UpdateAllPorts()
{
    const mode* merged_mode = GetMergedMode();
    bool        animated    = false;

    needs_full_update = false;

    if(merged_mode != NULL)
    {
        controller->SetMergedMode(*merged_mode);
        return;
    }

    if(controller->IsMerged())
    {
        controller->LeaveMergedMode();
    }

    for(unsigned int port = 0; port < UNIHUB_SLINF_GROUP_COUNT; port++)
    {
        animated |= UpdatePort(port);
    }

    if(animated)
    {
        controller->SendGlobalTrigger();
    }
}

bool RGBController_LianLiUniHubSLInfinity::UpdatePerLEDZone(unsigned int zone_idx)
{
    const mode* inner_mode;
    const mode* outer_mode;
    bool        outer_follows;

    GetPortModes(zone_idx / 2, &inner_mode, &outer_mode, &outer_follows);

    const mode* ring_mode = IsInnerChannel((int)zone_idx) ? inner_mode : outer_mode;

    if(ring_mode == NULL || ring_mode->color_mode != MODE_COLORS_PER_LED)
    {
        return(false);
    }

    return(UpdateRing(zone_idx, *ring_mode));
}

void RGBController_LianLiUniHubSLInfinity::DeviceUpdateLEDs()
{
    std::lock_guard<std::mutex> guard(update_mutex);

    bool animated = false;

    if(NeedsFullUpdate())
    {
        UpdateAllPorts();
        return;
    }

    for(unsigned int zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        animated |= UpdatePerLEDZone(zone_idx);
    }

    if(animated)
    {
        controller->SendGlobalTrigger();
    }
}

void RGBController_LianLiUniHubSLInfinity::DeviceUpdateZoneLEDs(int zone)
{
    std::lock_guard<std::mutex> guard(update_mutex);

    if((std::size_t)zone >= zones.size())
    {
        return;
    }

    if(NeedsFullUpdate())
    {
        UpdateAllPorts();
        return;
    }

    if(UpdatePerLEDZone((unsigned int)zone))
    {
        controller->SendGlobalTrigger();
    }
}

void RGBController_LianLiUniHubSLInfinity::DeviceUpdateSingleLED(int led)
{
    if((std::size_t)led < leds.size())
    {
        DeviceUpdateZoneLEDs(leds[led].value);
    }
}

/*---------------------------------------------------------*\
| Mode updates for the whole device send everything again,  |
| so that applying a mode or profile always fixes the hub   |
| state.                                                    |
\*---------------------------------------------------------*/
void RGBController_LianLiUniHubSLInfinity::DeviceUpdateMode()
{
    std::lock_guard<std::mutex> guard(update_mutex);

    controller->MarkAllChannelsStale();

    UpdateAllPorts();
}

void RGBController_LianLiUniHubSLInfinity::DeviceUpdateZoneMode(int zone)
{
    std::lock_guard<std::mutex> guard(update_mutex);

    if((std::size_t)zone >= zones.size() || zones[zone].leds_count == 0)
    {
        return;
    }

    if(NeedsFullUpdate())
    {
        UpdateAllPorts();
        return;
    }

    if(UpdatePort((unsigned int)zone / 2))
    {
        controller->SendGlobalTrigger();
    }
}
