/*---------------------------------------------------------*\
| RGBController_IlovbeeB87.cpp                             |
|                                                           |
|   RGBController for ilovbee B87 keyboard                  |
|                                                           |
|   w568w                                       18 Jul 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_IlovbeeB87.h"
#include "RGBControllerKeyNames.h"

typedef struct
{
    const char*     name;
    unsigned char   value;
    unsigned int    flags;
} ilovbee_b87_effect;

static const ilovbee_b87_effect ilovbee_b87_effects[] =
{
    { "Static",             ILOVBEE_B87_MODE_STATIC,             MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { "Breathing",          ILOVBEE_B87_MODE_BREATHING,          MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { "Rainbow",            ILOVBEE_B87_MODE_RAINBOW,            MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED },
    { "Flash Away",         ILOVBEE_B87_MODE_FLASH_AWAY,         MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { "Raindrops",          ILOVBEE_B87_MODE_RAINDROPS,          MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { "Rainbow Wheel",      ILOVBEE_B87_MODE_RAINBOW_WHEEL,      MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { "Ripples Shining",    ILOVBEE_B87_MODE_RIPPLES_SHINING,    MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { "Stars Twinkle",      ILOVBEE_B87_MODE_STARS_TWINKLE,      MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { "Shadow Disappear",   ILOVBEE_B87_MODE_SHADOW_DISAPPEAR,   MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { "Retro Snake",        ILOVBEE_B87_MODE_RETRO_SNAKE,        MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { "Neon Stream",        ILOVBEE_B87_MODE_NEON_STREAM,        MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { "Reaction",           ILOVBEE_B87_MODE_REACTION,           MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { "Sine Wave",          ILOVBEE_B87_MODE_SINE_WAVE,          MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR },
    { "Rotating Windmill",  ILOVBEE_B87_MODE_ROTATING_WINDMILL,  MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED },
    { "Colorful Waterfall", ILOVBEE_B87_MODE_WATERFALL,          MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED },
    { "Blossoming",         ILOVBEE_B87_MODE_BLOSSOMING,         MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_SPEED },
    { "Off",                ILOVBEE_B87_MODE_OFF,                 0 },
};

/**------------------------------------------------------------------*\
    @name ilovbee B87
    @category Keyboard
    @type USB
    @save :x:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectIlovbeeB87Controllers
    @comment Hardware effects only. Per-key custom mode is not supported.
\*-------------------------------------------------------------------*/

RGBController_IlovbeeB87::RGBController_IlovbeeB87(IlovbeeB87Controller* controller_ptr)
{
    controller  = controller_ptr;

    name        = controller->GetDeviceName();
    vendor      = "ilovbee";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "ilovbee B87 Keyboard";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();

    for(unsigned int effect_idx = 0; effect_idx < (sizeof(ilovbee_b87_effects) / sizeof(ilovbee_b87_effects[0])); effect_idx++)
    {
        const ilovbee_b87_effect& effect = ilovbee_b87_effects[effect_idx];
        mode new_mode;

        new_mode.name             = effect.name;
        new_mode.value            = effect.value;
        new_mode.flags            = effect.flags | MODE_FLAG_AUTOMATIC_SAVE;
        new_mode.color_mode       = MODE_COLORS_NONE;

        if(effect.flags & MODE_FLAG_HAS_BRIGHTNESS)
        {
            new_mode.brightness       = controller->GetModeBrightness(effect.value);
            new_mode.brightness_min   = ILOVBEE_B87_BRIGHTNESS_MIN;
            new_mode.brightness_max   = ILOVBEE_B87_BRIGHTNESS_MAX;
        }

        if(effect.flags & MODE_FLAG_HAS_SPEED)
        {
            new_mode.speed            = controller->GetModeSpeed(effect.value);
            new_mode.speed_min        = ILOVBEE_B87_SPEED_MIN;
            new_mode.speed_max        = ILOVBEE_B87_SPEED_MAX;
        }

        if(effect.flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
        {
            new_mode.color_mode   = MODE_COLORS_MODE_SPECIFIC;
            new_mode.colors_min   = 1;
            new_mode.colors_max   = 1;
            new_mode.colors.push_back(controller->GetModeColor(effect.value));
        }
        modes.push_back(new_mode);
    }

    SetupZones();

    active_mode = (int)modes.size() - 1;
    unsigned char current_mode = controller->GetCurrentMode();

    for(unsigned int mode_idx = 0; mode_idx < modes.size(); mode_idx++)
    {
        if(modes[mode_idx].value == current_mode)
        {
            active_mode = (int)mode_idx;
            break;
        }
    }
}

RGBController_IlovbeeB87::~RGBController_IlovbeeB87()
{
    Shutdown();

    delete controller;
}

void RGBController_IlovbeeB87::SetupZones()
{
    zone keyboard;
    keyboard.name       = ZONE_EN_KEYBOARD;
    keyboard.type       = ZONE_TYPE_SINGLE;
    keyboard.leds_min   = 1;
    keyboard.leds_max   = 1;
    keyboard.leds_count = 1;
    zones.push_back(keyboard);

    led keyboard_led;
    keyboard_led.name = ZONE_EN_KEYBOARD;
    leds.push_back(keyboard_led);

    SetupColors();
}

void RGBController_IlovbeeB87::DeviceUpdateMode()
{
    const mode& active = modes[active_mode];
    bool update_speed = (active.flags & MODE_FLAG_HAS_SPEED);
    bool update_color = (active.flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR);
    RGBColor color = update_color ? active.colors[0] : 0;

    controller->SetMode(active.value, active.brightness, update_speed, active.speed, update_color, color);
}
