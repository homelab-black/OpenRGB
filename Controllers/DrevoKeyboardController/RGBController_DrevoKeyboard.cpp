/*---------------------------------------------------------*\
| RGBController_DrevoKeyboard.cpp                           |
|                                                           |
|   RGBController for DREVO BladeMaster PRO keyboards       |
|                                                           |
|   T-bond                                      27 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <string>
#include "RGBControllerKeyNames.h"
#include "RGBController_DrevoKeyboard.h"

#define NA                                          0xFFFFFFFF

/*---------------------------------------------------------*\
| Physical grid of the 88 key ISO layout. [y][x] holds the  |
| key index, which is also the LED index within the zone    |
| and the position of the key in the per-key color packet.  |
| The ISO Enter spans rows 2 and 3, it is placed in row 2.  |
\*---------------------------------------------------------*/
#define DREVO_ISO_88_MATRIX_HEIGHT                  6
#define DREVO_ISO_88_MATRIX_WIDTH                   18

static unsigned int drevo_iso_88_matrix_map[DREVO_ISO_88_MATRIX_HEIGHT][DREVO_ISO_88_MATRIX_WIDTH] =
{
    {   0,  NA,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  NA,  13,  14,  15 },
    {  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  NA,  30,  31,  32 },
    {  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  NA,  47,  48,  49 },
    {  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  NA,  NA,  NA,  NA,  NA },
    {  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  NA,  75,  NA,  NA,  76,  NA },
    {  77,  78,  79,  NA,  NA,  NA,  80,  NA,  NA,  NA,  81,  82,  83,  84,  NA,  85,  86,  87 },
};

/*---------------------------------------------------------*\
| Key names in key index order                              |
\*---------------------------------------------------------*/
static const char* drevo_iso_88_key_names[DREVO_ISO_88_KEY_COUNT] =
{
    KEY_EN_ESCAPE,          KEY_EN_F1,              KEY_EN_F2,              KEY_EN_F3,
    KEY_EN_F4,              KEY_EN_F5,              KEY_EN_F6,              KEY_EN_F7,
    KEY_EN_F8,              KEY_EN_F9,              KEY_EN_F10,             KEY_EN_F11,
    KEY_EN_F12,             KEY_EN_PRINT_SCREEN,    KEY_EN_SCROLL_LOCK,     KEY_EN_PAUSE_BREAK,

    KEY_EN_BACK_TICK,       KEY_EN_1,               KEY_EN_2,               KEY_EN_3,
    KEY_EN_4,               KEY_EN_5,               KEY_EN_6,               KEY_EN_7,
    KEY_EN_8,               KEY_EN_9,               KEY_EN_0,               KEY_EN_MINUS,
    KEY_EN_EQUALS,          KEY_EN_BACKSPACE,       KEY_EN_INSERT,          KEY_EN_HOME,
    KEY_EN_PAGE_UP,

    KEY_EN_TAB,             KEY_EN_Q,               KEY_EN_W,               KEY_EN_E,
    KEY_EN_R,               KEY_EN_T,               KEY_EN_Y,               KEY_EN_U,
    KEY_EN_I,               KEY_EN_O,               KEY_EN_P,               KEY_EN_LEFT_BRACKET,
    KEY_EN_RIGHT_BRACKET,   KEY_EN_ISO_ENTER,       KEY_EN_DELETE,          KEY_EN_END,
    KEY_EN_PAGE_DOWN,

    KEY_EN_CAPS_LOCK,       KEY_EN_A,               KEY_EN_S,               KEY_EN_D,
    KEY_EN_F,               KEY_EN_G,               KEY_EN_H,               KEY_EN_J,
    KEY_EN_K,               KEY_EN_L,               KEY_EN_SEMICOLON,       KEY_EN_QUOTE,
    KEY_EN_POUND,

    KEY_EN_LEFT_SHIFT,      KEY_EN_ISO_BACK_SLASH,  KEY_EN_Z,               KEY_EN_X,
    KEY_EN_C,               KEY_EN_V,               KEY_EN_B,               KEY_EN_N,
    KEY_EN_M,               KEY_EN_COMMA,           KEY_EN_PERIOD,          KEY_EN_FORWARD_SLASH,
    KEY_EN_RIGHT_SHIFT,     KEY_EN_UP_ARROW,

    KEY_EN_LEFT_CONTROL,    KEY_EN_LEFT_WINDOWS,    KEY_EN_LEFT_ALT,        KEY_EN_SPACE,
    KEY_EN_RIGHT_ALT,       KEY_EN_RIGHT_WINDOWS,   KEY_EN_RIGHT_FUNCTION,  KEY_EN_RIGHT_CONTROL,
    KEY_EN_LEFT_ARROW,      KEY_EN_DOWN_ARROW,      KEY_EN_RIGHT_ARROW,
};

typedef struct
{
    const char*     name;
    unsigned char   value;
    unsigned int    flags;
} drevo_effect;

/**------------------------------------------------------------------*\
    @name DREVO BladeMaster PRO 88K
    @category Keyboard
    @type USB
    @save :robot:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectDrevoKeyboardControllers
    @comment The keyboard stores the lighting, including the per-key
             and light bar colors of the Custom mode, and keeps it
             after a power cycle.
\*-------------------------------------------------------------------*/

RGBController_DrevoKeyboard::RGBController_DrevoKeyboard(DrevoKeyboardController* controller_ptr)
{
    controller                  = controller_ptr;

    name                        = controller->GetNameString();
    vendor                      = "DREVO";
    type                        = DEVICE_TYPE_KEYBOARD;
    description                 = "DREVO BladeMaster PRO Keyboard";
    location                    = controller->GetDeviceLocation();
    serial                      = controller->GetSerialString();

    const drevo_effect drevo_effects[] =
    {
        { "Custom",             DREVO_MODE_CUSTOM,          MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS                                                                  },
        { "Static",             DREVO_MODE_STATIC,          MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS                               },
        { "Spectrum Cycle",     DREVO_MODE_SPECTRUM,        MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS                                                                          },
        { "Rainbow Wave",       DREVO_MODE_RAINBOW,         MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD                },
        { "Power Gauge",        DREVO_MODE_POWER_GAUGE,     MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS                                                                          },
        { "Breathing",          DREVO_MODE_BREATHING,       MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS         },
        { "Twinkling Stars",    DREVO_MODE_TWINKLING_STARS, MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS         },
        { "Reactive",           DREVO_MODE_REACTIVE,        MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS         },
        { "Marquee",            DREVO_MODE_MARQUEE,         MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS                                                                          },
        { "Aurora",             DREVO_MODE_AURORA,          MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS         },
        { "Off",                DREVO_MODE_OFF,             0                                                                                                                       },
    };

    for(const drevo_effect& effect : drevo_effects)
    {
        mode new_mode;

        new_mode.name           = effect.name;
        new_mode.value          = effect.value;
        new_mode.flags          = effect.flags | MODE_FLAG_AUTOMATIC_SAVE;

        if(new_mode.flags & MODE_FLAG_HAS_PER_LED_COLOR)
        {
            new_mode.color_mode = MODE_COLORS_PER_LED;
        }
        else if(new_mode.flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
        {
            /*---------------------------------------------*\
            | Random: the keyboard's own colors             |
            \*---------------------------------------------*/
            new_mode.color_mode = MODE_COLORS_MODE_SPECIFIC;
            new_mode.colors_min = 1;
            new_mode.colors_max = 1;
            new_mode.colors.resize(1, ToRGBColor(255, 255, 255));
        }
        else
        {
            new_mode.color_mode = MODE_COLORS_NONE;
        }

        if(new_mode.flags & MODE_FLAG_HAS_SPEED)
        {
            new_mode.speed_min  = DREVO_SPEED_MIN;
            new_mode.speed_max  = DREVO_SPEED_MAX;
            new_mode.speed      = (DREVO_SPEED_MIN + DREVO_SPEED_MAX) / 2;
        }

        if(new_mode.flags & MODE_FLAG_HAS_BRIGHTNESS)
        {
            new_mode.brightness_min = DREVO_BRIGHTNESS_MIN;
            new_mode.brightness_max = DREVO_BRIGHTNESS_MAX;
            new_mode.brightness     = DREVO_BRIGHTNESS_MAX;
        }

        if(new_mode.flags & MODE_FLAG_HAS_DIRECTION_LR)
        {
            new_mode.direction  = MODE_DIRECTION_LEFT;
        }

        modes.push_back(new_mode);
    }

    SetupZones();
}

RGBController_DrevoKeyboard::~RGBController_DrevoKeyboard()
{
    Shutdown();

    delete controller;
}

void RGBController_DrevoKeyboard::SetupZones()
{
    /*-----------------------------------------------------*\
    | Keyboard zone                                         |
    \*-----------------------------------------------------*/
    zone keyboard_zone;

    keyboard_zone.name          = ZONE_EN_KEYBOARD;
    keyboard_zone.type          = ZONE_TYPE_MATRIX;
    keyboard_zone.leds_min      = DREVO_ISO_88_KEY_COUNT;
    keyboard_zone.leds_max      = DREVO_ISO_88_KEY_COUNT;
    keyboard_zone.leds_count    = DREVO_ISO_88_KEY_COUNT;

    keyboard_zone.matrix_map.Set(DREVO_ISO_88_MATRIX_HEIGHT, DREVO_ISO_88_MATRIX_WIDTH, (unsigned int*)&drevo_iso_88_matrix_map);

    zones.push_back(keyboard_zone);

    for(unsigned int led_idx = 0; led_idx < DREVO_ISO_88_KEY_COUNT; led_idx++)
    {
        led new_led;

        new_led.name            = drevo_iso_88_key_names[led_idx];
        new_led.value           = led_idx;

        leds.push_back(new_led);
    }

    /*-----------------------------------------------------*\
    | Light bar zone: the side LEDs around the keyboard,    |
    | one segment per side; top and bottom run left to      |
    | right, left and right bottom to top                   |
    \*-----------------------------------------------------*/
    const struct
    {
        const char*     name;
        unsigned int    count;
    } light_bar_sides[] =
    {
        { "Top",    DREVO_LIGHT_BAR_TOP_COUNT       },
        { "Right",  DREVO_LIGHT_BAR_RIGHT_COUNT     },
        { "Bottom", DREVO_LIGHT_BAR_BOTTOM_COUNT    },
        { "Left",   DREVO_LIGHT_BAR_LEFT_COUNT      },
    };

    zone light_bar_zone;

    light_bar_zone.name         = "Light Bar";
    light_bar_zone.type         = ZONE_TYPE_LINEAR;
    light_bar_zone.leds_min     = DREVO_LIGHT_BAR_COUNT;
    light_bar_zone.leds_max     = DREVO_LIGHT_BAR_COUNT;
    light_bar_zone.leds_count   = DREVO_LIGHT_BAR_COUNT;

    unsigned int start_idx      = 0;

    for(const auto& side : light_bar_sides)
    {
        segment side_segment;

        side_segment.name       = side.name;
        side_segment.type       = ZONE_TYPE_LINEAR;
        side_segment.start_idx  = start_idx;
        side_segment.leds_count = side.count;

        light_bar_zone.segments.push_back(side_segment);

        for(unsigned int led_idx = 0; led_idx < side.count; led_idx++)
        {
            led new_led;

            new_led.name        = std::string("Light Bar ") + side.name + " " + std::to_string(led_idx + 1);
            new_led.value       = DREVO_ISO_88_KEY_COUNT + start_idx + led_idx;

            leds.push_back(new_led);
        }

        start_idx              += side.count;
    }

    zones.push_back(light_bar_zone);

    SetupColors();
}

void RGBController_DrevoKeyboard::DeviceUpdateLEDs()
{
    if(modes[active_mode].value == DREVO_MODE_CUSTOM)
    {
        controller->SetLEDs(colors);
    }
}

void RGBController_DrevoKeyboard::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_DrevoKeyboard::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_DrevoKeyboard::DeviceUpdateMode()
{
    const mode& current_mode = modes[active_mode];

    if(current_mode.value == DREVO_MODE_OFF)
    {
        controller->SetMode(DREVO_MODE_STATIC, true, 0, 0, ToRGBColor(0, 0, 0), 0);
        return;
    }

    bool            use_color   = (current_mode.color_mode == MODE_COLORS_MODE_SPECIFIC);
    RGBColor        color       = use_color ? current_mode.colors[0] : ToRGBColor(0, 0, 0);
    unsigned char   speed       = (current_mode.flags & MODE_FLAG_HAS_SPEED) ? current_mode.speed : 0;
    unsigned char   direction   = DREVO_RAINBOW_RIGHT_TO_LEFT;

    switch(current_mode.direction)
    {
        case MODE_DIRECTION_LEFT:
            direction = DREVO_RAINBOW_RIGHT_TO_LEFT;
            break;

        case MODE_DIRECTION_RIGHT:
            direction = DREVO_RAINBOW_LEFT_TO_RIGHT;
            break;

        case MODE_DIRECTION_UP:
            direction = DREVO_RAINBOW_DOWN_TO_UP;
            break;

        case MODE_DIRECTION_DOWN:
            direction = DREVO_RAINBOW_UP_TO_DOWN;
            break;
    }

    controller->SetMode(current_mode.value, use_color, speed, current_mode.brightness, color, direction);

    if(current_mode.value == DREVO_MODE_CUSTOM)
    {
        controller->SetLEDs(colors);
    }
}
