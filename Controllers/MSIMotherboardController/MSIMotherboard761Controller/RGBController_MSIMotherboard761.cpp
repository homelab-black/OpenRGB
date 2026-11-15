/*---------------------------------------------------------*\
| RGBController_MSIMotherboard761.cpp                       |
|                                                           |
|   RGBController for MSI Mystic Light 761-byte motherboard |
|                                                           |
|                                                           |
|   rom4ster                                    11 Jun 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_MSIMotherboard761.h"
#include "LogManager.h"

struct ZoneDescription
{
    std::string name;
    MSI_ZONE    zone_type;
};

#define NUMOF_ZONES     (sizeof(led_zones) / sizeof(ZoneDescription))

const ZoneDescription led_zones[] =
{
    ZoneDescription{ "JAF",         MSI_ZONE_JAF     },
    ZoneDescription{ "JARGB 1",     MSI_ZONE_JARGB_1 },
    ZoneDescription{ "JARGB 2",     MSI_ZONE_JARGB_2 },
    ZoneDescription{ "JARGB 3",     MSI_ZONE_JARGB_3 },
    ZoneDescription{ "IO Shield",   MSI_ZONE_J_PIPE_1 },
};

static std::vector<const ZoneDescription*> zone_description;

static int IndexOfZoneForType(MSI_ZONE zone_type)
{
    for(std::size_t i = 0; i < zone_description.size(); ++i)
    {
        if(zone_description[i]->zone_type == zone_type)
        {
            return (int) i;
        }
    }

    return -1;
}

RGBController_MSIMotherboard761::RGBController_MSIMotherboard761
    (
    MSIMotherboard761Controller* controller_ptr
    )
{
    controller  = controller_ptr;

    name        = controller->GetDeviceName();
    vendor      = "MSI";
    type        = DEVICE_TYPE_MOTHERBOARD;
    description = "MSI Mystic Light Device (761-byte)";
    version     = controller->GetFWVersion();
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerial();

    const std::vector<MSI_ZONE>* supported_zones = controller->GetSupportedZones();

    for(std::size_t i = 0; i < supported_zones->size(); ++i)
    {
        for(std::size_t j = 0; j < NUMOF_ZONES; ++j)
        {
            if(led_zones[j].zone_type == (*supported_zones)[i])
            {
                zone_description.push_back(&led_zones[j]);
                break;
            }
        }
    }

    last_resizable_zone = MSI_ZONE_NONE;
    SetupModes();
    SetupZones();
    active_mode         = GetDeviceMode();
}

RGBController_MSIMotherboard761::~RGBController_MSIMotherboard761()
{
    Shutdown();

    zone_description.clear();
    delete controller;
}

int RGBController_MSIMotherboard761::GetDeviceMode()
{
    int mode = controller->GetMode();

    for(std::size_t i = 0; i < modes.size(); ++i)
    {
        if(mode == modes[i].value)
        {
            return (int)i;
        }
    }

    return 0;
}

void RGBController_MSIMotherboard761::SetupZones()
{
    /*-----------------------------------------------------*\
    | Only set LED count on the first run                   |
    \*-----------------------------------------------------*/
    bool first_run = false;

    if(zones.size() == 0)
    {
        first_run = true;
    }

    /*-----------------------------------------------------*\
    | Clear any existing color/LED configuration            |
    \*-----------------------------------------------------*/
    leds.clear();
    colors.clear();
    zones.resize(zone_description.size());

    /*-----------------------------------------------------*\
    | Set up zones                                          |
    \*-----------------------------------------------------*/
    for(std::size_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        const ZoneDescription*  zd                      = zone_description[zone_idx];
        unsigned int            maxLeds                 = (unsigned int)controller->GetMaxDirectLeds(zd->zone_type);

        /*-------------------------------------------------*\
        | This is a fixed size zone                         |
        \*-------------------------------------------------*/
        if(((zd->zone_type != MSI_ZONE_JAF)
         && (zd->zone_type != MSI_ZONE_JARGB_1)
         && (zd->zone_type != MSI_ZONE_JARGB_2)
         && (zd->zone_type != MSI_ZONE_JARGB_3)))
        {
            zones[zone_idx].name                        = zd->name;
            zones[zone_idx].type                        = ZONE_TYPE_SINGLE;
            zones[zone_idx].flags                       = 0;
            zones[zone_idx].leds_min                    = maxLeds;
            zones[zone_idx].leds_max                    = maxLeds;
            zones[zone_idx].leds_count                  = maxLeds;
        }
        /*--------------------------------------------------\
        | This is a resizable zone on a board that does not |
        | support per-LED direct mode                       |
        \*-------------------------------------------------*/
        else if(controller->GetSupportedDirectMode() == MSIMotherboard761Controller::DIRECT_MODE_ZONE_BASED)
        {
            zones[zone_idx].name                        = zd->name;
            zones[zone_idx].type                        = ZONE_TYPE_SINGLE;
            zones[zone_idx].leds_min                    = 0;
            zones[zone_idx].leds_max                    = 30;//maxLeds;

            if(first_run)
            {
                zones[zone_idx].flags                   = ZONE_FLAG_MANUALLY_CONFIGURABLE_SIZE_EFFECTS_ONLY;
            }

            if(!(zones[zone_idx].flags & ZONE_FLAG_MANUALLY_CONFIGURED_SIZE))
            {
                zones[zone_idx].leds_count              = 0;
            }

            last_resizable_zone                         = zd->zone_type;
        }
        /*--------------------------------------------------\
        | This is a resizable zone on a board that does     |
        | support per-LED direct mode                       |
        \*-------------------------------------------------*/
        else
        {
            zones[zone_idx].leds_min                    = 0;
            zones[zone_idx].leds_max                    = maxLeds;

            if(first_run)
            {
                zones[zone_idx].flags                   = ZONE_FLAG_MANUALLY_CONFIGURABLE_SIZE
                                                        | ZONE_FLAG_MANUALLY_CONFIGURABLE_NAME
                                                        | ZONE_FLAG_MANUALLY_CONFIGURABLE_TYPE
                                                        | ZONE_FLAG_MANUALLY_CONFIGURABLE_MATRIX_MAP
                                                        | ZONE_FLAG_MANUALLY_CONFIGURABLE_SEGMENTS;
            }

            if(!(zones[zone_idx].flags & ZONE_FLAG_MANUALLY_CONFIGURED_NAME))
            {
                zones[zone_idx].name                    = zd->name;
            }

            if(!(zones[zone_idx].flags & ZONE_FLAG_MANUALLY_CONFIGURED_SIZE))
            {
                /*-------------------------------------------------*\
                | A hardware effect only spreads over the number of |
                | LEDs named in the row's count byte, so size these |
                | headers the way MSI Center does rather than 0.    |
                \*-------------------------------------------------*/
                zones[zone_idx].leds_count              = controller->SupportsHardwareModes()
                                                        ? controller->GetZoneLedNum(zd->zone_type)
                                                        : 0;
            }

            if(!(zones[zone_idx].flags & ZONE_FLAG_MANUALLY_CONFIGURED_TYPE))
            {
                zones[zone_idx].type                    = ZONE_TYPE_LINEAR;
            }

            if(!(zones[zone_idx].flags & ZONE_FLAG_MANUALLY_CONFIGURED_MATRIX_MAP))
            {
                zones[zone_idx].matrix_map.width        = 0;
                zones[zone_idx].matrix_map.height       = 0;
                zones[zone_idx].matrix_map.map.resize(0);
            }

            last_resizable_zone                         = zd->zone_type;
        }

        controller->SetZoneLEDCount(zone_description[zone_idx]->zone_type, zones[zone_idx].leds_count);

        if((zones[zone_idx].flags & ZONE_FLAG_MANUALLY_CONFIGURABLE_SIZE_EFFECTS_ONLY) == 0)
        {
            for(std::size_t led_idx = 0; led_idx < zones[zone_idx].leds_count; led_idx++)
            {
                led new_led;
                new_led.name                            = zones[zone_idx].name + ", LED " + std::to_string(led_idx + 1);
                new_led.value                           = zone_description[zone_idx]->zone_type;
                leds.push_back(new_led);
            }
        }
        else if(zones[zone_idx].leds_count > 0)
        {
            led new_led;
            new_led.name                                = zones[zone_idx].name;
            new_led.value                               = zone_description[zone_idx]->zone_type;
            leds.push_back(new_led);
        }
    }

    controller->SendConfiguration();

    SetupColors();
}


void RGBController_MSIMotherboard761::DeviceConfigureZone(int zone_idx)
{
    if((std::size_t)zone_idx < zones.size())
    {
        SetupZones();

        if(zone_description[zone_idx]->zone_type == last_resizable_zone)
        {
            GetDeviceConfig();
            last_resizable_zone = MSI_ZONE_NONE;
        }
    }
}

void RGBController_MSIMotherboard761::DeviceUpdateLEDs()
{
    /*-----------------------------------------------------*\
    | OpenRGB keeps one active mode per device, so an       |
    | effect is written into every zone row even though the |
    | report carries a row per header and could hold one    |
    | effect each.  This is how the other MSI motherboard   |
    | drivers behave too; giving the headers their own      |
    | effects would mean mapping modes onto zones.          |
    \*-----------------------------------------------------*/
    for(std::size_t zone_idx = 0; zone_idx < zones.size(); ++zone_idx)
    {
        for(int led_idx = zones[zone_idx].leds_count - 1; led_idx >= 0; led_idx--)
        {
            UpdateLed((int)zone_idx, led_idx);
        }
    }
    controller->Update((modes[active_mode].flags & MODE_FLAG_AUTOMATIC_SAVE) != 0);
}

void RGBController_MSIMotherboard761::DeviceUpdateZoneLEDs(int zone)
{
    for(int led_idx = zones[zone].leds_count - 1; led_idx >= 0; led_idx--)
    {
        UpdateLed(zone, led_idx);
    }
    controller->Update((modes[active_mode].flags & MODE_FLAG_AUTOMATIC_SAVE) != 0);
}

void RGBController_MSIMotherboard761::DeviceUpdateSingleLED
    (
        int led
        )
{
    int zone_index = IndexOfZoneForType((MSI_ZONE)leds[led].value);

    if(zone_index == -1)
    {
        LOG_DEBUG("[%s]: could not find zone for type %d", controller->GetDeviceName().c_str(), leds[led].value);
        return;
    }

    int led_index = led - zones[zone_index].start_idx;
    UpdateLed(zone_index, led_index);
    controller->Update((modes[active_mode].flags & MODE_FLAG_AUTOMATIC_SAVE) != 0);
}

void RGBController_MSIMotherboard761::DeviceUpdateMode()
{
    if(modes[active_mode].value == MSI_MODE_DIRECT_DUMMY)
    {
        controller->SetDirectMode(true);
    }
    else
    {
        controller->SetDirectMode(false);
        DeviceUpdateLEDs();
    }
}

void RGBController_MSIMotherboard761::DeviceSaveMode()
{
    controller->Update(true);
}

void RGBController_MSIMotherboard761::SetupModes()
{
    constexpr unsigned int SAVED  = MODE_FLAG_MANUAL_SAVE;
    constexpr unsigned int PARAMS = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED;

    if(controller->GetSupportedDirectMode() != MSIMotherboard761Controller::DIRECT_MODE_DISABLED)
    {
        SetupMode("Direct",                 MSI_MODE_DIRECT_DUMMY,                  MODE_FLAG_HAS_PER_LED_COLOR, 0, 0);
    }

    /*-----------------------------------------------------*\
    | Effect presets of the 0x50 report's zone rows.  The   |
    | names follow MSI Center, whose capture identified     |
    | every ID that is offered here.  Mapping is always by  |
    | name, never by number, because this report's numbering|
    | differs from the older MSI protocols.                 |
    |                                                       |
    | ID 0x07 is not offered: it left every LED dark even   |
    | when MSI Center's own frame was replayed unchanged.   |
    \*-----------------------------------------------------*/
    if(controller->SupportsHardwareModes())
    {
        /*-------------------------------------------------*\
        | Names follow the MSI_MODE vocabulary the other    |
        | MSI motherboard drivers use; the MSI Center label |
        | for each ID is in the controller's table.         |
        |                                                   |
        | Color source per capture:  Fire/Lightning use the |
        | row's palette,  Color ring was only ever written  |
        | with generated colors,  Wave and Meteor do either |
        | (MSI Center writes the generated form unless a    |
        | color preset is chosen), so those advertise both. |
        |  Wave honouring a custom palette was confirmed on |
        | hardware; whether Color ring does the same is not |
        | established, so it stays generated-only.          |
        |                                                   |
        | Direction is offered where the capture shows the  |
        | bit toggling: Wave and Color ring.                |
        \*-------------------------------------------------*/
        SetupMode("Off",                    MSI_MODE_DISABLE,                       SAVED, 0, 0);
        SetupMode("Static",                 MSI_MODE_STATIC,                        SAVED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_PER_LED_COLOR, 0, 0);
        SetupMode("Breathing",              MSI_MODE_BREATHING,                     SAVED | PARAMS | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_PER_LED_COLOR, 0, 0);
        SetupMode("Fire",                   MSI_MODE_FIRE,                          SAVED | PARAMS | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_PER_LED_COLOR, 1, MSI_50_NUM_PALETTE_COLORS);
        SetupMode("Color wave",             MSI_MODE_COLOR_WAVE,                    SAVED | PARAMS | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR, 1, MSI_50_NUM_PALETTE_COLORS);
        SetupMode("Color ring",             MSI_MODE_COLOR_RING,                    SAVED | PARAMS | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR, 1, MSI_50_NUM_PALETTE_COLORS);
        SetupMode("Lightning",              MSI_MODE_LIGHTNING,                     SAVED | PARAMS | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_PER_LED_COLOR, 1, MSI_50_NUM_PALETTE_COLORS);
        SetupMode("Meteor",                 MSI_MODE_METEOR,                        SAVED | PARAMS | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR, 1, MSI_50_NUM_PALETTE_COLORS);
    }
}

void RGBController_MSIMotherboard761::UpdateLed
    (
    int zone,
    int led
    )
{
    unsigned char red = RGBGetRValue(zones[zone].colors[led]);
    unsigned char grn = RGBGetGValue(zones[zone].colors[led]);
    unsigned char blu = RGBGetBValue(zones[zone].colors[led]);

    if(controller->IsDirectModeActive())
    {
        controller->SetLedColor((MSI_ZONE)(zones[zone].leds[led].value), led, red, grn, blu);
    }
    else
    {
        if(led == 0)
        {
            bool           random     = modes[active_mode].color_mode == MODE_COLORS_RANDOM;
            int            mode       = modes[active_mode].value;
            MSI_SPEED      speed      = (MSI_SPEED)modes[active_mode].speed;
            MSI_BRIGHTNESS brightness = (MSI_BRIGHTNESS)modes[active_mode].brightness;
            MSI_ZONE       msi_zone   = (MSI_ZONE)zones[zone].leds[led].value;

            /*-------------------------------------------------*\
            | Direction is only offered for modes that carry   |
            | the flag.  OpenRGB's default direction value is  |
            | LEFT, so the flag must be checked before using   |
            | it, or every mode would ask for the direction    |
            | bit to be set.                                   |
            \*-------------------------------------------------*/
            bool left = ((modes[active_mode].flags & MODE_FLAG_HAS_DIRECTION_LR)
                      && (modes[active_mode].direction == MODE_DIRECTION_LEFT));

            controller->SetMode(msi_zone, mode, speed, brightness, random, left);

            /*-------------------------------------------------*\
            | Presets that cycle a palette take their colors   |
            | from the mode's color list (up to 4 slots); the  |
            | others use the zone's first LED color.           |
            \*-------------------------------------------------*/
            if((modes[active_mode].color_mode == MODE_COLORS_MODE_SPECIFIC)
             && (modes[active_mode].colors.size() > 0))
            {
                Color        palette[MSI_50_NUM_PALETTE_COLORS] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
                unsigned int num_colors                         = 0;

                for(std::size_t i = 0; i < modes[active_mode].colors.size(); i++)
                {
                    if(num_colors >= MSI_50_NUM_PALETTE_COLORS)
                    {
                        break;
                    }

                    palette[num_colors].R = RGBGetRValue(modes[active_mode].colors[i]);
                    palette[num_colors].G = RGBGetGValue(modes[active_mode].colors[i]);
                    palette[num_colors].B = RGBGetBValue(modes[active_mode].colors[i]);
                    num_colors++;
                }

                /*-------------------------------------------------*\
                | The row always carries four slots, so repeat the  |
                | last given color into the unused ones; otherwise  |
                | a single-color choice would still cycle through   |
                | whatever the row held before.                     |
                \*-------------------------------------------------*/
                if((num_colors > 0) && (num_colors < MSI_50_NUM_PALETTE_COLORS))
                {
                    for(unsigned int i = num_colors; i < MSI_50_NUM_PALETTE_COLORS; i++)
                    {
                        palette[i] = palette[num_colors - 1];
                    }

                    num_colors = MSI_50_NUM_PALETTE_COLORS;
                }

                controller->SetZonePalette(msi_zone, palette, num_colors);
            }
            else
            {
                controller->SetZoneColor(msi_zone, red, grn, blu, red, grn, blu);
            }
        }
    }
}

void RGBController_MSIMotherboard761::SetupMode
    (
    const char      *name,
    int             mod,
    unsigned int    flags,
    unsigned int    colors_min,
    unsigned int    colors_max
    )
{
    mode Mode;
    Mode.name   = name;
    Mode.value  = mod;
    Mode.flags  = flags;

    if(flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
    {
        /*-----------------------------------------------------*\
        | Start the palette off with the same                 |
        | red/green/blue/white colors MSI Center leaves in the|
        | rows, not white in every slot: an untouched palette |
        | then still shows a recognizable effect instead of a |
        | steady white one.  Colors are packed 0x00BBGGRR.    |
        |                                                     |
        | Every one of MSI Center's 67 captured frames that   |
        | uses the row palette fills all four slots with      |
        | different colors; the only frames with identical    |
        | slots are the generated-color form, where the       |
        | palette is cleared.  Its Flame row carries          |
        | red/orange/blue/white.  Picking one color here      |
        | repeats it into all four slots (OpenRGB convention),|
        | which left Fire looking steady on MS-7E47: the      |
        | vendor never writes that shape, so a preset needs   |
        | distinct slots to look animated.                    |
        \*-----------------------------------------------------*/
        static const RGBColor default_palette[MSI_50_NUM_PALETTE_COLORS] =
        {
            ToRGBColor(255,   0,   0),
            ToRGBColor(  0, 255,   0),
            ToRGBColor(  0,   0, 255),
            ToRGBColor(255, 255, 255),
        };

        Mode.color_mode = MODE_COLORS_MODE_SPECIFIC;
        Mode.colors_min = colors_min;
        Mode.colors_max = colors_max;
        Mode.colors.resize(colors_max);

        for(unsigned int i = 0; i < colors_max; i++)
        {
            Mode.colors[i] = default_palette[i % MSI_50_NUM_PALETTE_COLORS];
        }
    }
    else if(flags & MODE_FLAG_HAS_PER_LED_COLOR)
    {
        Mode.color_mode = MODE_COLORS_PER_LED;
    }
    else
    {
        Mode.color_mode = MODE_COLORS_RANDOM;
    }

    if(flags & MODE_FLAG_HAS_SPEED)
    {
        Mode.speed      = MSI_SPEED_MEDIUM;
        Mode.speed_max  = MSI_SPEED_HIGH;
        Mode.speed_min  = MSI_SPEED_LOW;
    }
    else
    {
        /*---------------------------------------------------------*\
        | For modes without speed this needs to be set to avoid     |
        | bad values in the saved profile which in turn corrupts    |
        | the brightness calculation when loading the profile       |
        \*---------------------------------------------------------*/
        Mode.speed      = 0;
        Mode.speed_max  = 0;
        Mode.speed_min  = 0;
    }

    if(flags & MODE_FLAG_HAS_BRIGHTNESS)
    {
        Mode.brightness      = MSI_BRIGHTNESS_LEVEL_100;
        Mode.brightness_max  = MSI_BRIGHTNESS_LEVEL_100;
        Mode.brightness_min  = MSI_BRIGHTNESS_OFF;
    }
    else
    {
        Mode.brightness      = MSI_BRIGHTNESS_LEVEL_100;
        Mode.brightness_max  = MSI_BRIGHTNESS_LEVEL_100;
        Mode.brightness_min  = MSI_BRIGHTNESS_LEVEL_100;
    }

    modes.push_back(Mode);
}

void RGBController_MSIMotherboard761::GetDeviceConfig()
{
    if(controller->GetMode() != MSI_MODE_DIRECT_DUMMY)
    {
        int            mode;
        MSI_SPEED      speed;
        MSI_BRIGHTNESS brightness;
        bool           rainbow;
        unsigned int   color;

        for(std::size_t i = 0; i < zone_description.size(); ++i)
        {
            controller->GetMode(zone_description[i]->zone_type, mode, speed, brightness, rainbow, color);

            if(zones[i].colors != nullptr)
            {
                for(size_t j = 0; j < LEDsInZone((unsigned int)i); ++j)
                {
                    zones[i].colors[j] = color;
                }
            }
        }

        controller->GetMode(zone_description[0]->zone_type, mode, speed, brightness, rainbow, color);

        for(std::size_t i = 0; i < modes.size(); ++i)
        {
            if(mode == modes[i].value)
            {
                if(modes[i].flags & MODE_FLAG_HAS_SPEED)
                {
                    modes[i].speed = speed;
                }
                if(modes[i].flags & MODE_FLAG_HAS_BRIGHTNESS)
                {
                    modes[i].brightness = brightness;
                }
                if(rainbow)
                {
                    if(modes[i].flags & (MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_RANDOM_COLOR))
                    {
                        if(rainbow)
                        {
                            modes[i].color_mode = MODE_COLORS_RANDOM;
                        }
                        else
                        {
                            modes[i].color_mode = MODE_COLORS_PER_LED;
                        }
                    }
                }
                break;
            }
        }
    }
}
