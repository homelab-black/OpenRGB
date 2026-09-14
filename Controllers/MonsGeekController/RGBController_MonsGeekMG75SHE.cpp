/*---------------------------------------------------------*\
| RGBController_MonsGeekMG75SHE.cpp                         |
|                                                           |
|   RGBController for the MonsGeek MG75S HE keyboard        |
|                                                           |
|   Sergey Tsai (mustang651)                 14 Sep 2026    |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBControllerKeyNames.h"
#include "RGBController_MonsGeekMG75SHE.h"

#define NA                                      0xFFFFFFFF
#define MONSGEEK_MG75S_HE_MATRIX_HEIGHT         6
#define MONSGEEK_MG75S_HE_MATRIX_WIDTH          17

static unsigned int monsgeek_mg75s_he_matrix_map[MONSGEEK_MG75S_HE_MATRIX_HEIGHT][MONSGEEK_MG75S_HE_MATRIX_WIDTH] =
{
    {  0,  1,  2,  3,  4,  5,  6,  7,  8, NA,  9, 10, 11, 12, 13, NA, 14 },
    { 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, NA, NA, 29 },
    { 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, NA, NA, 44 },
    { 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, NA, NA, NA, 58 },
    { 59, NA, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, NA, 79, NA, 71 },
    { 72, 73, 74, NA, 75, NA, NA, NA, NA, NA, 76, 77, 78, 80, 81, 82, NA }
};

static const char* monsgeek_mg75s_he_led_names[] =
{
    KEY_EN_ESCAPE,
    KEY_EN_F1,
    KEY_EN_F2,
    KEY_EN_F3,
    KEY_EN_F4,
    KEY_EN_F5,
    KEY_EN_F6,
    KEY_EN_F7,
    KEY_EN_F8,
    KEY_EN_F9,
    KEY_EN_F10,
    KEY_EN_F11,
    KEY_EN_F12,
    KEY_EN_DELETE,
    KEY_EN_INSERT,
    KEY_EN_BACK_TICK,
    KEY_EN_1,
    KEY_EN_2,
    KEY_EN_3,
    KEY_EN_4,
    KEY_EN_5,
    KEY_EN_6,
    KEY_EN_7,
    KEY_EN_8,
    KEY_EN_9,
    KEY_EN_0,
    KEY_EN_MINUS,
    KEY_EN_EQUALS,
    KEY_EN_BACKSPACE,
    KEY_EN_HOME,
    KEY_EN_TAB,
    KEY_EN_Q,
    KEY_EN_W,
    KEY_EN_E,
    KEY_EN_R,
    KEY_EN_T,
    KEY_EN_Y,
    KEY_EN_U,
    KEY_EN_I,
    KEY_EN_O,
    KEY_EN_P,
    KEY_EN_LEFT_BRACKET,
    KEY_EN_RIGHT_BRACKET,
    KEY_EN_BACK_SLASH,
    KEY_EN_PAGE_UP,
    KEY_EN_CAPS_LOCK,
    KEY_EN_A,
    KEY_EN_S,
    KEY_EN_D,
    KEY_EN_F,
    KEY_EN_G,
    KEY_EN_H,
    KEY_EN_J,
    KEY_EN_K,
    KEY_EN_L,
    KEY_EN_SEMICOLON,
    KEY_EN_QUOTE,
    KEY_EN_ANSI_ENTER,
    KEY_EN_PAGE_DOWN,
    KEY_EN_LEFT_SHIFT,
    KEY_EN_Z,
    KEY_EN_X,
    KEY_EN_C,
    KEY_EN_V,
    KEY_EN_B,
    KEY_EN_N,
    KEY_EN_M,
    KEY_EN_COMMA,
    KEY_EN_PERIOD,
    KEY_EN_FORWARD_SLASH,
    KEY_EN_RIGHT_SHIFT,
    KEY_EN_END,
    KEY_EN_LEFT_CONTROL,
    KEY_EN_LEFT_WINDOWS,
    KEY_EN_LEFT_ALT,
    KEY_EN_SPACE,
    KEY_EN_RIGHT_ALT,
    KEY_EN_RIGHT_FUNCTION,
    KEY_EN_RIGHT_CONTROL,
    KEY_EN_UP_ARROW,
    KEY_EN_LEFT_ARROW,
    KEY_EN_DOWN_ARROW,
    KEY_EN_RIGHT_ARROW
};

static const unsigned char monsgeek_mg75s_he_hardware_indices[] =
{
     0,  6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72, 78, 84,
     1,  7, 13, 19, 25, 31, 37, 43, 49, 55, 61, 67, 73, 79, 85,
     2,  8, 14, 20, 26, 32, 38, 44, 50, 56, 62, 68, 74, 80, 86,
     3,  9, 15, 21, 27, 33, 39, 45, 51, 57, 63, 69, 81, 87,
     4, 16, 22, 28, 34, 40, 46, 52, 58, 64, 70, 76, 88,
     5, 17, 23, 41, 59, 65, 71, 82, 77, 83, 89
};

static_assert
    (
    sizeof(monsgeek_mg75s_he_led_names) / sizeof(monsgeek_mg75s_he_led_names[0]) == MONSGEEK_MG75S_HE_LED_COUNT,
    "The MG75S HE LED name table must contain one entry per LED"
    );

static_assert
    (
    sizeof(monsgeek_mg75s_he_hardware_indices) / sizeof(monsgeek_mg75s_he_hardware_indices[0]) == MONSGEEK_MG75S_HE_LED_COUNT,
    "The MG75S HE hardware index table must contain one entry per LED"
    );

/**------------------------------------------------------------------*\
    @name MonsGeek MG75S HE
    @category Keyboard
    @type USB
    @save :robot:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectMonsGeekMG75SHEControllers
    @comment
\*---------------------------------------------------------*/

RGBController_MonsGeekMG75SHE::RGBController_MonsGeekMG75SHE(MonsGeekMG75SHEController* controller)
{
    this->controller           = controller;
    this->static_mode_prepared = false;

    name        = controller->GetDeviceName();
    vendor      = "MonsGeek";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "MonsGeek MG75S HE Hall Effect Keyboard";
    serial      = controller->GetSerial();
    location    = controller->GetLocation();

    mode Static;
    Static.name            = "Static";
    Static.value           = MONSGEEK_MG75S_HE_MODE_USER_PICTURE;
    Static.flags           = MODE_FLAG_HAS_BRIGHTNESS
                           | MODE_FLAG_HAS_PER_LED_COLOR
                           | MODE_FLAG_AUTOMATIC_SAVE;
    Static.brightness_min  = MONSGEEK_MG75S_HE_BRIGHTNESS_MIN;
    Static.brightness_max  = MONSGEEK_MG75S_HE_BRIGHTNESS_MAX;
    Static.brightness      = MONSGEEK_MG75S_HE_BRIGHTNESS_DEFAULT;
    Static.color_mode      = MODE_COLORS_PER_LED;
    modes.push_back(Static);

    mode Off;
    Off.name               = "Off";
    Off.value              = MONSGEEK_MG75S_HE_MODE_OFF;
    Off.flags              = 0;
    Off.color_mode         = MODE_COLORS_NONE;
    modes.push_back(Off);

    const unsigned int colored_effect_flags = MODE_FLAG_HAS_SPEED
                                             | MODE_FLAG_HAS_BRIGHTNESS
                                             | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR
                                             | MODE_FLAG_HAS_RANDOM_COLOR;

    AddEffect("Solid",        MONSGEEK_MG75S_HE_MODE_SOLID,        MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR);
    AddEffect("Breathing",    MONSGEEK_MG75S_HE_MODE_BREATHING,    colored_effect_flags);
    AddEffect("Spectrum",     MONSGEEK_MG75S_HE_MODE_SPECTRUM,     MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS);
    AddEffect("Wave",         MONSGEEK_MG75S_HE_MODE_WAVE,         colored_effect_flags | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD);
    AddEffect("Ripple",       MONSGEEK_MG75S_HE_MODE_RIPPLE,       colored_effect_flags);
    AddEffect("Rain",         MONSGEEK_MG75S_HE_MODE_RAIN,         colored_effect_flags);
    AddEffect("Snake",        MONSGEEK_MG75S_HE_MODE_SNAKE,        colored_effect_flags);
    AddEffect("Reactive",     MONSGEEK_MG75S_HE_MODE_REACTIVE,     colored_effect_flags);
    AddEffect("Convergence",  MONSGEEK_MG75S_HE_MODE_CONVERGENCE,  colored_effect_flags);
    AddEffect("Sine Wave",    MONSGEEK_MG75S_HE_MODE_SINE_WAVE,    colored_effect_flags);
    AddEffect("Kaleidoscope", MONSGEEK_MG75S_HE_MODE_KALEIDOSCOPE, colored_effect_flags);
    AddEffect("Line Wave",    MONSGEEK_MG75S_HE_MODE_LINE_WAVE,    colored_effect_flags | MODE_FLAG_HAS_DIRECTION_LR);
    AddEffect("Laser",        MONSGEEK_MG75S_HE_MODE_LASER,        colored_effect_flags);
    AddEffect("Circle Wave",  MONSGEEK_MG75S_HE_MODE_CIRCLE_WAVE,  colored_effect_flags | MODE_FLAG_HAS_DIRECTION_LR, MODE_DIRECTION_LEFT);
    AddEffect("Dazzling",     MONSGEEK_MG75S_HE_MODE_DAZZLING,     colored_effect_flags);
    AddEffect("Rain Down",    MONSGEEK_MG75S_HE_MODE_RAIN_DOWN,    colored_effect_flags);
    AddEffect("Meteor",       MONSGEEK_MG75S_HE_MODE_METEOR,       colored_effect_flags);
    AddEffect("Reactive Off", MONSGEEK_MG75S_HE_MODE_REACTIVE_OFF, colored_effect_flags);
    AddEffect("Train",        MONSGEEK_MG75S_HE_MODE_TRAIN,        colored_effect_flags);
    AddEffect("Fireworks",    MONSGEEK_MG75S_HE_MODE_FIREWORKS,    colored_effect_flags | MODE_FLAG_HAS_DIRECTION_LR);

    SetupZones();
}

RGBController_MonsGeekMG75SHE::~RGBController_MonsGeekMG75SHE()
{
    Shutdown();
    delete controller;
}

void RGBController_MonsGeekMG75SHE::AddEffect
    (
    const char*     effect_name,
    int             effect_value,
    unsigned int    effect_flags,
    unsigned int    default_direction
    )
{
    mode effect;

    effect.name            = effect_name;
    effect.value           = effect_value;
    effect.flags           = effect_flags | MODE_FLAG_REQUIRES_ENTIRE_DEVICE;
    effect.speed_min       = MONSGEEK_MG75S_HE_SPEED_MIN;
    effect.speed_max       = MONSGEEK_MG75S_HE_SPEED_MAX;
    effect.speed           = MONSGEEK_MG75S_HE_SPEED_DEFAULT;
    effect.brightness_min  = MONSGEEK_MG75S_HE_BRIGHTNESS_MIN;
    effect.brightness_max  = MONSGEEK_MG75S_HE_BRIGHTNESS_MAX;
    effect.brightness      = MONSGEEK_MG75S_HE_BRIGHTNESS_DEFAULT;
    effect.direction       = default_direction;

    if(effect_flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
    {
        effect.colors_min = 1;
        effect.colors_max = 1;
        effect.colors.resize(1);
        effect.colors[0]  = ToRGBColor(255, 0, 0);
        effect.color_mode = MODE_COLORS_MODE_SPECIFIC;
    }
    else
    {
        effect.color_mode = MODE_COLORS_NONE;
    }

    modes.push_back(effect);
}

void RGBController_MonsGeekMG75SHE::SetupZones()
{
    zone keyboard_zone;

    keyboard_zone.name       = ZONE_EN_KEYBOARD;
    keyboard_zone.type       = ZONE_TYPE_MATRIX;
    keyboard_zone.leds_min   = MONSGEEK_MG75S_HE_LED_COUNT;
    keyboard_zone.leds_max   = MONSGEEK_MG75S_HE_LED_COUNT;
    keyboard_zone.leds_count = MONSGEEK_MG75S_HE_LED_COUNT;
    keyboard_zone.matrix_map.Set
        (
        MONSGEEK_MG75S_HE_MATRIX_HEIGHT,
        MONSGEEK_MG75S_HE_MATRIX_WIDTH,
        &monsgeek_mg75s_he_matrix_map[0][0]
        );

    zones.push_back(keyboard_zone);

    for(unsigned int led_index = 0; led_index < MONSGEEK_MG75S_HE_LED_COUNT; led_index++)
    {
        led keyboard_led;

        keyboard_led.name  = monsgeek_mg75s_he_led_names[led_index];
        keyboard_led.value = monsgeek_mg75s_he_hardware_indices[led_index];
        leds.push_back(keyboard_led);
    }

    SetupColors();
}

void RGBController_MonsGeekMG75SHE::DeviceUpdateLEDs()
{
    std::lock_guard<std::mutex> guard(device_mutex);

    if(modes[active_mode].value == MONSGEEK_MG75S_HE_MODE_USER_PICTURE)
    {
        if(!EnsureStaticMode())
        {
            return;
        }

        if(!controller->SetStatic(colors.data(), monsgeek_mg75s_he_hardware_indices, colors.size()))
        {
            static_mode_prepared = false;
            return;
        }

        /*-------------------------------------------------*\
        | A keyboard shortcut can change the hardware mode. |
        | Prepare Static again before the next saved update. |
        \*-------------------------------------------------*/
        static_mode_prepared = false;
    }
}

void RGBController_MonsGeekMG75SHE::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_MonsGeekMG75SHE::DeviceUpdateSingleLED(int led_index)
{
    std::lock_guard<std::mutex> guard(device_mutex);

    if(modes[active_mode].value == MONSGEEK_MG75S_HE_MODE_USER_PICTURE
    &&  led_index >= 0
    &&  (unsigned int)led_index < leds.size())
    {
        if(!EnsureStaticMode())
        {
            return;
        }

        if(!controller->SetStaticLED(leds[led_index].value, colors[led_index]))
        {
            static_mode_prepared = false;
            return;
        }

        static_mode_prepared = false;
    }
}

bool RGBController_MonsGeekMG75SHE::EnsureStaticMode()
{
    if(static_mode_prepared)
    {
        return(true);
    }

    static_mode_prepared = controller->SetMode
        (
        MONSGEEK_MG75S_HE_MODE_USER_PICTURE,
        0,
        modes[active_mode].brightness,
        0,
        0
        );

    return(static_mode_prepared);
}

void RGBController_MonsGeekMG75SHE::DeviceUpdateMode()
{
    std::lock_guard<std::mutex> guard(device_mutex);
    mode& active = modes[active_mode];

    if(active.value == MONSGEEK_MG75S_HE_MODE_USER_PICTURE)
    {
        static_mode_prepared = controller->SetMode
            (
            MONSGEEK_MG75S_HE_MODE_USER_PICTURE,
            0,
            active.brightness,
            0,
            0
            );
        return;
    }

    static_mode_prepared = false;

    unsigned char option = MONSGEEK_MG75S_HE_OPTION_SOLID;

    if(active.color_mode == MODE_COLORS_RANDOM)
    {
        option = MONSGEEK_MG75S_HE_OPTION_RANDOM;
    }

    if(active.value == MONSGEEK_MG75S_HE_MODE_SPECTRUM
    || active.value == MONSGEEK_MG75S_HE_MODE_OFF)
    {
        option = MONSGEEK_MG75S_HE_OPTION_RANDOM;
    }

    if(active.value == MONSGEEK_MG75S_HE_MODE_WAVE)
    {
        if(active.direction == MODE_DIRECTION_LEFT)
        {
            option |= 0x10;
        }
        else if(active.direction == MODE_DIRECTION_DOWN)
        {
            option |= 0x20;
        }
        else if(active.direction == MODE_DIRECTION_UP)
        {
            option |= 0x30;
        }
    }
    else if(active.value == MONSGEEK_MG75S_HE_MODE_LINE_WAVE
         || active.value == MONSGEEK_MG75S_HE_MODE_FIREWORKS)
    {
        if(active.direction == MODE_DIRECTION_LEFT)
        {
            option |= 0x10;
        }
    }
    else if(active.value == MONSGEEK_MG75S_HE_MODE_CIRCLE_WAVE)
    {
        if(active.direction == MODE_DIRECTION_RIGHT)
        {
            option |= 0x10;
        }
    }

    RGBColor color = ToRGBColor(255, 0, 0);

    if(!active.colors.empty())
    {
        color = active.colors[0];
    }

    unsigned char speed = (active.flags & MODE_FLAG_HAS_SPEED) ? active.speed : 0;

    controller->SetMode
        (
        active.value,
        speed,
        active.brightness,
        option,
        color
        );
}
