/*---------------------------------------------------------*\
| RGBController_EVisionV3Keyboard.cpp                       |
|                                                           |
|   RGBController for EVision "RDR" tri-mode keyboards      |
|                                                           |
|   Jan Imrich (imrija)                         05 Jul 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "hsv.h"
#include "RGBController_EVisionV3Keyboard.h"

/*---------------------------------------------------------*\
| Effect values as exposed on the VIA RGB matrix channel.   |
| The list matches the original EVision effect table        |
| (see EVisionKeyboardController.h), shifted to start at 0. |
\*---------------------------------------------------------*/
enum
{
    EVISION_V3_EFFECT_OFF                   = 0x00,
    EVISION_V3_EFFECT_COLOR_WAVE_SHORT      = 0x01,
    EVISION_V3_EFFECT_COLOR_WAVE_LONG       = 0x02,
    EVISION_V3_EFFECT_COLOR_WHEEL           = 0x03,
    EVISION_V3_EFFECT_SPECTRUM_CYCLE        = 0x04,
    EVISION_V3_EFFECT_BREATHING             = 0x05,
    EVISION_V3_EFFECT_STATIC                = 0x06,
    EVISION_V3_EFFECT_REACTIVE              = 0x07,
    EVISION_V3_EFFECT_REACTIVE_RIPPLE       = 0x08,
    EVISION_V3_EFFECT_REACTIVE_LINE         = 0x09,
    EVISION_V3_EFFECT_STARLIGHT_FAST        = 0x0A,
    EVISION_V3_EFFECT_BLOOMING              = 0x0B,
    EVISION_V3_EFFECT_RAINBOW_VERTICAL      = 0x0C,
    EVISION_V3_EFFECT_HURRICANE             = 0x0D,
    EVISION_V3_EFFECT_ACCUMULATE            = 0x0E,
    EVISION_V3_EFFECT_STARLIGHT_SLOW        = 0x0F,
    EVISION_V3_EFFECT_VISOR                 = 0x10,
    EVISION_V3_EFFECT_SURMOUNT              = 0x11,
    EVISION_V3_EFFECT_RAINBOW_CIRCLE        = 0x12,
};

struct evision_v3_effect
{
    const char*     name;
    unsigned char   value;
    bool            has_speed;
    bool            has_color;
};

static const evision_v3_effect effect_table[] =
{
    { "Off",                EVISION_V3_EFFECT_OFF,              false,  false },
    { "Static",             EVISION_V3_EFFECT_STATIC,           false,  true  },
    { "Color Wave (Short)", EVISION_V3_EFFECT_COLOR_WAVE_SHORT, true,   true  },
    { "Color Wave",         EVISION_V3_EFFECT_COLOR_WAVE_LONG,  true,   true  },
    { "Color Wheel",        EVISION_V3_EFFECT_COLOR_WHEEL,      true,   true  },
    { "Spectrum Cycle",     EVISION_V3_EFFECT_SPECTRUM_CYCLE,   true,   false },
    { "Breathing",          EVISION_V3_EFFECT_BREATHING,        true,   true  },
    { "Reactive",           EVISION_V3_EFFECT_REACTIVE,         true,   true  },
    { "Reactive Ripple",    EVISION_V3_EFFECT_REACTIVE_RIPPLE,  true,   true  },
    { "Reactive Line",      EVISION_V3_EFFECT_REACTIVE_LINE,    true,   true  },
    { "Starlight",          EVISION_V3_EFFECT_STARLIGHT_FAST,   true,   true  },
    { "Blooming",           EVISION_V3_EFFECT_BLOOMING,         true,   false },
    { "Vertical Rainbow",   EVISION_V3_EFFECT_RAINBOW_VERTICAL, true,   true  },
    { "Hurricane",          EVISION_V3_EFFECT_HURRICANE,        true,   true  },
    { "Accumulate",         EVISION_V3_EFFECT_ACCUMULATE,       true,   true  },
    { "Starlight (Slow)",   EVISION_V3_EFFECT_STARLIGHT_SLOW,   true,   true  },
    { "Visor",              EVISION_V3_EFFECT_VISOR,            false,  true  },
    { "Surmount",           EVISION_V3_EFFECT_SURMOUNT,         true,   true  },
    { "Rainbow Circle",     EVISION_V3_EFFECT_RAINBOW_CIRCLE,   false,  false },
};

/**------------------------------------------------------------------*\
    @name EVision V3 Keyboard
    @category Keyboard
    @type USB
    @save :white_check_mark:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectEVisionV3Keyboards
    @comment The EVision "RDR" tri-mode keyboard firmware exposes its
        effects over the VIA protocol RGB matrix channel.  Sold under
        many brands (Eternico, Yenkee, Womier, Zuoya, IQUNIX, ...).
\*-------------------------------------------------------------------*/

RGBController_EVisionV3Keyboard::RGBController_EVisionV3Keyboard(EVisionV3KeyboardController* controller_ptr)
{
    controller  = controller_ptr;

    name        = controller->GetName();
    vendor      = "EVision";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "EVision VIA Keyboard Device";
    location    = controller->GetLocation();
    serial      = controller->GetSerial();
    version     = controller->GetVersion();

    for(unsigned int effect_idx = 0; effect_idx < (sizeof(effect_table) / sizeof(effect_table[0])); effect_idx++)
    {
        const evision_v3_effect& effect = effect_table[effect_idx];

        mode new_mode;

        new_mode.name           = effect.name;
        new_mode.value          = effect.value;
        new_mode.flags          = MODE_FLAG_MANUAL_SAVE;
        new_mode.color_mode     = MODE_COLORS_NONE;

        if(effect.value != EVISION_V3_EFFECT_OFF)
        {
            new_mode.flags         |= MODE_FLAG_HAS_BRIGHTNESS;
            new_mode.brightness_min = EVISION_V3_BRIGHTNESS_MIN;
            new_mode.brightness_max = EVISION_V3_BRIGHTNESS_MAX;
            new_mode.brightness     = EVISION_V3_BRIGHTNESS_MAX;
        }

        if(effect.has_speed)
        {
            new_mode.flags         |= MODE_FLAG_HAS_SPEED;
            new_mode.speed_min      = EVISION_V3_SPEED_MIN;
            new_mode.speed_max      = EVISION_V3_SPEED_MAX;
            new_mode.speed          = (EVISION_V3_SPEED_MAX / 2);
        }

        if(effect.has_color)
        {
            new_mode.flags         |= MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
            new_mode.colors_min     = 1;
            new_mode.colors_max     = 1;
            new_mode.color_mode     = MODE_COLORS_MODE_SPECIFIC;
            new_mode.colors.resize(1);
        }

        modes.push_back(new_mode);
    }

    SetupZones();

    /*-----------------------------------------------------*\
    | The firmware answers reads, so initialize the UI from |
    | the device's current state                            |
    \*-----------------------------------------------------*/
    ReadDeviceState();
}

RGBController_EVisionV3Keyboard::~RGBController_EVisionV3Keyboard()
{
    delete controller;
}

void RGBController_EVisionV3Keyboard::SetupZones()
{
    /*-----------------------------------------------------*\
    | The firmware only exposes hardware effects over the   |
    | VIA channel, so represent the keyboard as one zone    |
    | with a single logical LED                             |
    \*-----------------------------------------------------*/
    zone keyboard_zone;

    keyboard_zone.name          = "Keyboard";
    keyboard_zone.type          = ZONE_TYPE_SINGLE;
    keyboard_zone.leds_min      = 1;
    keyboard_zone.leds_max      = 1;
    keyboard_zone.leds_count    = 1;
    keyboard_zone.matrix_map    = NULL;

    zones.push_back(keyboard_zone);

    led keyboard_led;

    keyboard_led.name           = "Keyboard";

    leds.push_back(keyboard_led);

    SetupColors();
}

void RGBController_EVisionV3Keyboard::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*-----------------------------------------------------*\
    | This device does not support resizing zones           |
    \*-----------------------------------------------------*/
}

void RGBController_EVisionV3Keyboard::DeviceUpdateLEDs()
{
    /*-----------------------------------------------------*\
    | This device does not support per LED control          |
    \*-----------------------------------------------------*/
}

void RGBController_EVisionV3Keyboard::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_EVisionV3Keyboard::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_EVisionV3Keyboard::DeviceUpdateMode()
{
    unsigned char hue       = 0;
    unsigned char sat       = 0;
    bool          set_color = (modes[active_mode].color_mode == MODE_COLORS_MODE_SPECIFIC);

    if(set_color && modes[active_mode].colors.size() > 0)
    {
        hsv_t hsv_color;

        rgb2hsv(modes[active_mode].colors[0], &hsv_color);

        hue = (unsigned char)((float)hsv_color.hue * (256.0f / 360.0f));
        sat = hsv_color.saturation;
    }

    controller->SetMode((unsigned char)modes[active_mode].value,
                        (unsigned char)modes[active_mode].speed,
                        hue,
                        sat,
                        (unsigned char)modes[active_mode].brightness,
                        set_color);
}

void RGBController_EVisionV3Keyboard::DeviceSaveMode()
{
    controller->SaveMode();
}

void RGBController_EVisionV3Keyboard::ReadDeviceState()
{
    unsigned char effect        = controller->GetEffect();
    unsigned char brightness    = controller->GetBrightness();
    unsigned char speed         = controller->GetSpeed();
    unsigned char hue           = 0;
    unsigned char sat           = 0;

    controller->GetColor(&hue, &sat);

    for(unsigned int mode_idx = 0; mode_idx < modes.size(); mode_idx++)
    {
        if(modes[mode_idx].value == effect)
        {
            active_mode = mode_idx;

            if(modes[mode_idx].flags & MODE_FLAG_HAS_BRIGHTNESS)
            {
                modes[mode_idx].brightness = brightness;
            }

            if(modes[mode_idx].flags & MODE_FLAG_HAS_SPEED)
            {
                modes[mode_idx].speed = speed;
            }

            if(modes[mode_idx].color_mode == MODE_COLORS_MODE_SPECIFIC)
            {
                hsv_t hsv_color;

                hsv_color.hue        = (int)((float)hue * (360.0f / 256.0f));
                hsv_color.saturation = sat;
                hsv_color.value      = 255;

                modes[mode_idx].colors[0] = hsv2rgb(&hsv_color);
            }

            break;
        }
    }
}
