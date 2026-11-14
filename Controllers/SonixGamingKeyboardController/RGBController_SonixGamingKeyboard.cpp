/*---------------------------------------------------------*\
| RGBController_SonixGamingKeyboard.cpp                     |
|                                                           |
|   RGBController for Sonix based keyboards using the       |
|   "Gaming Force Z" software (CZC.Gaming Revenant)         |
|                                                           |
|   Roman Ondráček                              05 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBControllerKeyNames.h"
#include "RGBController_SonixGamingKeyboard.h"

#define NA                  0xFFFFFFFF
#define MATRIX_HEIGHT       6
#define MATRIX_WIDTH        23

struct sonix_gk_key
{
    const char*     name;
    unsigned int    matrix_row;     /* Row in the OpenRGB matrix map        */
    unsigned int    matrix_col;     /* Column in the OpenRGB matrix map     */
    unsigned int    device_row;     /* Row in the keyboard's LED matrix     */
    unsigned int    device_col;     /* Column in the keyboard's LED matrix  */
};

/*---------------------------------------------------------*\
| Full size ANSI layout (104 keys) of the CZC.Gaming        |
| Revenant GK990, taken from the vendor software's          |
| LightingAdvUI.xml.  Used for all keyboards with this ID   |
| until other layouts are added.                            |
\*---------------------------------------------------------*/
static const sonix_gk_key ansi_104_keys[] =
{
    {  KEY_EN_ESCAPE,               0,  0, 0,  1 },
    {  KEY_EN_F1,                   0,  2, 0,  2 },
    {  KEY_EN_F2,                   0,  3, 0,  3 },
    {  KEY_EN_F3,                   0,  4, 0,  4 },
    {  KEY_EN_F4,                   0,  5, 0,  5 },
    {  KEY_EN_F5,                   0,  6, 0,  6 },
    {  KEY_EN_F6,                   0,  7, 0,  7 },
    {  KEY_EN_F7,                   0,  8, 0,  8 },
    {  KEY_EN_F8,                   0,  9, 0,  9 },
    {  KEY_EN_F9,                   0, 11, 0, 10 },
    {  KEY_EN_F10,                  0, 12, 0, 11 },
    {  KEY_EN_F11,                  0, 13, 0, 12 },
    {  KEY_EN_F12,                  0, 14, 0, 13 },
    {  KEY_EN_PRINT_SCREEN,         0, 15, 6,  4 },
    {  KEY_EN_SCROLL_LOCK,          0, 16, 6,  5 },
    {  KEY_EN_PAUSE_BREAK,          0, 17, 6,  7 },
    {  KEY_EN_BACK_TICK,            1,  0, 1,  1 },
    {  KEY_EN_1,                    1,  1, 1,  2 },
    {  KEY_EN_2,                    1,  2, 1,  3 },
    {  KEY_EN_3,                    1,  3, 1,  4 },
    {  KEY_EN_4,                    1,  4, 1,  5 },
    {  KEY_EN_5,                    1,  5, 1,  6 },
    {  KEY_EN_6,                    1,  6, 1,  7 },
    {  KEY_EN_7,                    1,  7, 1,  8 },
    {  KEY_EN_8,                    1,  8, 1,  9 },
    {  KEY_EN_9,                    1,  9, 1, 10 },
    {  KEY_EN_0,                    1, 10, 1, 11 },
    {  KEY_EN_MINUS,                1, 11, 1, 12 },
    {  KEY_EN_EQUALS,               1, 12, 1, 13 },
    {  KEY_EN_BACKSPACE,            1, 13, 5, 13 },
    {  KEY_EN_INSERT,               1, 15, 6,  8 },
    {  KEY_EN_HOME,                 1, 16, 6,  9 },
    {  KEY_EN_PAGE_UP,              1, 17, 6, 10 },
    {  KEY_EN_NUMPAD_LOCK,          1, 19, 1, 14 },
    {  KEY_EN_NUMPAD_DIVIDE,        1, 20, 1, 15 },
    {  KEY_EN_NUMPAD_TIMES,         1, 21, 1, 16 },
    {  KEY_EN_NUMPAD_MINUS,         1, 22, 6, 14 },
    {  KEY_EN_TAB,                  2,  0, 2,  1 },
    {  KEY_EN_Q,                    2,  2, 2,  2 },
    {  KEY_EN_W,                    2,  3, 2,  3 },
    {  KEY_EN_E,                    2,  4, 2,  4 },
    {  KEY_EN_R,                    2,  5, 2,  5 },
    {  KEY_EN_T,                    2,  6, 2,  6 },
    {  KEY_EN_Y,                    2,  7, 2,  7 },
    {  KEY_EN_U,                    2,  8, 2,  8 },
    {  KEY_EN_I,                    2,  9, 2,  9 },
    {  KEY_EN_O,                    2, 10, 2, 10 },
    {  KEY_EN_P,                    2, 11, 2, 11 },
    {  KEY_EN_LEFT_BRACKET,         2, 12, 2, 12 },
    {  KEY_EN_RIGHT_BRACKET,        2, 13, 2, 13 },
    {  KEY_EN_ANSI_BACK_SLASH,      2, 14, 3, 13 },
    {  KEY_EN_DELETE,               2, 15, 6, 11 },
    {  KEY_EN_END,                  2, 16, 6, 12 },
    {  KEY_EN_PAGE_DOWN,            2, 17, 6, 13 },
    {  KEY_EN_NUMPAD_7,             2, 19, 2, 14 },
    {  KEY_EN_NUMPAD_8,             2, 20, 2, 15 },
    {  KEY_EN_NUMPAD_9,             2, 21, 2, 16 },
    {  KEY_EN_NUMPAD_PLUS,          2, 22, 6, 15 },
    {  KEY_EN_CAPS_LOCK,            3,  0, 3,  1 },
    {  KEY_EN_A,                    3,  2, 3,  2 },
    {  KEY_EN_S,                    3,  3, 3,  3 },
    {  KEY_EN_D,                    3,  4, 3,  4 },
    {  KEY_EN_F,                    3,  5, 3,  5 },
    {  KEY_EN_G,                    3,  6, 3,  6 },
    {  KEY_EN_H,                    3,  7, 3,  7 },
    {  KEY_EN_J,                    3,  8, 3,  8 },
    {  KEY_EN_K,                    3,  9, 3,  9 },
    {  KEY_EN_L,                    3, 10, 3, 10 },
    {  KEY_EN_SEMICOLON,            3, 11, 3, 11 },
    {  KEY_EN_QUOTE,                3, 12, 3, 12 },
    {  KEY_EN_ANSI_ENTER,           3, 13, 4, 13 },
    {  KEY_EN_NUMPAD_4,             3, 19, 3, 14 },
    {  KEY_EN_NUMPAD_5,             3, 20, 3, 15 },
    {  KEY_EN_NUMPAD_6,             3, 21, 3, 16 },
    {  KEY_EN_LEFT_SHIFT,           4,  1, 4,  1 },
    {  KEY_EN_Z,                    4,  2, 4,  2 },
    {  KEY_EN_X,                    4,  3, 4,  3 },
    {  KEY_EN_C,                    4,  4, 4,  4 },
    {  KEY_EN_V,                    4,  5, 4,  5 },
    {  KEY_EN_B,                    4,  6, 4,  6 },
    {  KEY_EN_N,                    4,  7, 4,  7 },
    {  KEY_EN_M,                    4,  8, 4,  8 },
    {  KEY_EN_COMMA,                4,  9, 4,  9 },
    {  KEY_EN_PERIOD,               4, 10, 4, 10 },
    {  KEY_EN_FORWARD_SLASH,        4, 11, 4, 11 },
    {  KEY_EN_RIGHT_SHIFT,          4, 13, 4, 12 },
    {  KEY_EN_UP_ARROW,             4, 16, 5, 11 },
    {  KEY_EN_NUMPAD_1,             4, 19, 4, 14 },
    {  KEY_EN_NUMPAD_2,             4, 20, 4, 15 },
    {  KEY_EN_NUMPAD_3,             4, 21, 4, 16 },
    {  KEY_EN_NUMPAD_ENTER,         4, 22, 5, 16 },
    {  KEY_EN_LEFT_CONTROL,         5,  0, 5,  1 },
    {  KEY_EN_LEFT_WINDOWS,         5,  1, 5,  2 },
    {  KEY_EN_LEFT_ALT,             5,  3, 5,  3 },
    {  KEY_EN_SPACE,                5,  6, 5,  4 },
    {  KEY_EN_RIGHT_ALT,            5, 10, 5,  5 },
    {  KEY_EN_RIGHT_FUNCTION,       5, 11, 5,  6 },
    {  KEY_EN_MENU,                 5, 13, 5,  7 },
    {  KEY_EN_RIGHT_CONTROL,        5, 14, 5,  8 },
    {  KEY_EN_LEFT_ARROW,           5, 15, 5,  9 },
    {  KEY_EN_DOWN_ARROW,           5, 16, 5, 10 },
    {  KEY_EN_RIGHT_ARROW,          5, 17, 5, 12 },
    {  KEY_EN_NUMPAD_0,             5, 19, 5, 14 },
    {  KEY_EN_NUMPAD_PERIOD,        5, 21, 5, 15 },
};

struct sonix_gk_mode_info
{
    const char*     name;
    unsigned char   value;
    unsigned char   direction;      /* Default direction from vendor config */
    bool            random;         /* Default "full color" flag            */
};

static const sonix_gk_mode_info builtin_modes[SONIX_GK_BUILTIN_MODE_COUNT] =
{
    { "Static",         SONIX_GK_MODE_STATIC,       3,  true    },
    { "Single On",      SONIX_GK_MODE_SINGLE_ON,    0,  true    },
    { "Single Off",     SONIX_GK_MODE_SINGLE_OFF,   0,  true    },
    { "Glittering",     SONIX_GK_MODE_GLITTERING,   0,  true    },
    { "Falling",        SONIX_GK_MODE_FALLING,      0,  false   },
    { "Colourful",      SONIX_GK_MODE_COLOURFUL,    0,  true    },
    { "Breathing",      SONIX_GK_MODE_BREATH,       0,  false   },
    { "Spectrum",       SONIX_GK_MODE_SPECTRUM,     0,  true    },
    { "Outward",        SONIX_GK_MODE_OUTWARD,      0,  true    },
    { "Scrolling",      SONIX_GK_MODE_SCROLLING,    3,  true    },
    { "Rolling",        SONIX_GK_MODE_ROLLING,      0,  true    },
    { "Rotating",       SONIX_GK_MODE_ROTATING,     0,  true    },
    { "Explode",        SONIX_GK_MODE_EXPLODE,      0,  true    },
    { "Launch",         SONIX_GK_MODE_LAUNCH,       0,  true    },
    { "Ripples",        SONIX_GK_MODE_RIPPLES,      0,  true    },
    { "Flowing",        SONIX_GK_MODE_FLOWING,      0,  true    },
    { "Pulsating",      SONIX_GK_MODE_PULSATING,    0,  true    },
    { "Tilt",           SONIX_GK_MODE_TILT,         0,  true    },
    { "Shuttle",        SONIX_GK_MODE_SHUTTLE,      0,  true    },
};

/**------------------------------------------------------------------*\
    @name Sonix Gaming Keyboard
    @category Keyboard
    @type USB
    @save :robot:
    @direct :x:
    @effects :white_check_mark:
    @detectors DetectSonixGamingKeyboards
    @comment Keyboards using the Sonix "Gaming Force Z" software. The
    vendor reuses 0C45:8006 for many keyboards, tested with the CZC.Gaming
    Revenant GK990, whose 104 key ANSI layout is used for all of them.
    Every change is written to the keyboard's flash memory, so there is
    no Direct mode. The Revenant's bottom light bar runs its own effect and
    does not follow the lighting data, so it is not exposed.
\*-------------------------------------------------------------------*/

RGBController_SonixGamingKeyboard::RGBController_SonixGamingKeyboard(SonixGamingKeyboardController* controller_ptr)
{
    controller                      = controller_ptr;

    name                            = controller->GetNameString();
    vendor                          = "Sonix";
    type                            = DEVICE_TYPE_KEYBOARD;
    description                     = "Sonix Gaming Keyboard Device";
    location                        = controller->GetDeviceLocation();
    serial                          = controller->GetSerialString();

    mode Custom;
    Custom.name                     = "Custom";
    Custom.value                    = SONIX_GK_MODE_CUSTOM;
    Custom.flags                    = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_AUTOMATIC_SAVE;
    Custom.brightness_min           = SONIX_GK_BRIGHTNESS_MIN;
    Custom.brightness_max           = SONIX_GK_BRIGHTNESS_MAX;
    Custom.brightness               = SONIX_GK_BRIGHTNESS_MAX;
    Custom.color_mode               = MODE_COLORS_PER_LED;
    modes.push_back(Custom);

    for(unsigned int mode_idx = 0; mode_idx < SONIX_GK_BUILTIN_MODE_COUNT; mode_idx++)
    {
        mode new_mode;
        new_mode.name               = builtin_modes[mode_idx].name;
        new_mode.value              = builtin_modes[mode_idx].value;
        new_mode.flags              = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_AUTOMATIC_SAVE;
        new_mode.speed_min          = SONIX_GK_SPEED_MIN;
        new_mode.speed_max          = SONIX_GK_SPEED_MAX;
        new_mode.speed              = SONIX_GK_SPEED_DEFAULT;
        new_mode.brightness_min     = SONIX_GK_BRIGHTNESS_MIN;
        new_mode.brightness_max     = SONIX_GK_BRIGHTNESS_MAX;
        new_mode.brightness         = SONIX_GK_BRIGHTNESS_MAX;
        new_mode.colors_min         = 1;
        new_mode.colors_max         = 1;
        new_mode.color_mode         = builtin_modes[mode_idx].random ? MODE_COLORS_RANDOM : MODE_COLORS_MODE_SPECIFIC;
        new_mode.colors.resize(1, ToRGBColor(0x00, 0x00, 0xFF));
        modes.push_back(new_mode);
    }

    SetupZones();
}

RGBController_SonixGamingKeyboard::~RGBController_SonixGamingKeyboard()
{
    Shutdown();

    delete controller;
}

void RGBController_SonixGamingKeyboard::SetupZones()
{
    const unsigned int led_count = sizeof(ansi_104_keys) / sizeof(ansi_104_keys[0]);

    zone new_zone;
    new_zone.name                   = "Keyboard";
    new_zone.type                   = ZONE_TYPE_MATRIX;
    new_zone.leds_min               = led_count;
    new_zone.leds_max               = led_count;
    new_zone.leds_count             = led_count;

    unsigned int map[MATRIX_HEIGHT * MATRIX_WIDTH];

    for(unsigned int map_idx = 0; map_idx < (MATRIX_HEIGHT * MATRIX_WIDTH); map_idx++)
    {
        map[map_idx]            = NA;
    }

    for(unsigned int led_idx = 0; led_idx < led_count; led_idx++)
    {
        map[ansi_104_keys[led_idx].matrix_row * MATRIX_WIDTH + ansi_104_keys[led_idx].matrix_col] = led_idx;

        led new_led;
        new_led.name                = ansi_104_keys[led_idx].name;
        new_led.value               = ansi_104_keys[led_idx].device_row * SONIX_GK_MATRIX_COLUMNS + ansi_104_keys[led_idx].device_col;
        leds.push_back(new_led);
    }

    new_zone.matrix_map.Set(MATRIX_HEIGHT, MATRIX_WIDTH, map);
    zones.push_back(new_zone);

    SetupColors();
}

void RGBController_SonixGamingKeyboard::SendLighting()
{
    sonix_gk_mode_record    records[SONIX_GK_CUSTOM_RECORD_INDEX + 1] = {};
    RGBColor                key_colors[SONIX_GK_MATRIX_ROWS * SONIX_GK_MATRIX_COLUMNS];
    unsigned int            active = SONIX_GK_CUSTOM_RECORD_INDEX;

    /*-----------------------------------------------------*    | The keyboard stores the settings of every mode, so    |
    | send the current settings of all modes                |
    \*-----------------------------------------------------*/
    for(unsigned int mode_idx = 0; mode_idx < modes.size(); mode_idx++)
    {
        const mode&             m       = modes[mode_idx];
        unsigned int            index   = (m.value == SONIX_GK_MODE_CUSTOM) ? SONIX_GK_CUSTOM_RECORD_INDEX : (m.value - 1);
        sonix_gk_mode_record&   record  = records[index];
        RGBColor                color   = m.colors.empty() ? 0 : m.colors[0];

        record.mode         = m.value;
        record.red          = RGBGetRValue(color);
        record.green        = RGBGetGValue(color);
        record.blue         = RGBGetBValue(color);
        record.random       = (m.color_mode == MODE_COLORS_RANDOM);
        record.brightness   = m.brightness;
        record.speed        = m.speed;
        record.direction    = (index < SONIX_GK_BUILTIN_MODE_COUNT) ? builtin_modes[index].direction : 0;

        if((int)mode_idx == active_mode)
        {
            active = index;
        }
    }

    for(unsigned int key_idx = 0; key_idx < (SONIX_GK_MATRIX_ROWS * SONIX_GK_MATRIX_COLUMNS); key_idx++)
    {
        key_colors[key_idx] = 0;
    }

    for(unsigned int led_idx = 0; led_idx < leds.size(); led_idx++)
    {
        key_colors[leds[led_idx].value] = colors[led_idx];
    }

    controller->SetLighting(records, SONIX_GK_CUSTOM_RECORD_INDEX + 1, active, key_colors);
}

void RGBController_SonixGamingKeyboard::DeviceUpdateLEDs()
{
    SendLighting();
}

void RGBController_SonixGamingKeyboard::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SonixGamingKeyboard::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_SonixGamingKeyboard::DeviceUpdateMode()
{
    SendLighting();
}
