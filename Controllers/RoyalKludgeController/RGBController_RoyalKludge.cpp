/*---------------------------------------------------------*\
| RGBController_RoyalKludge.cpp                             |
|                                                           |
|   RGBController for Royal Kludge RK987 keyboards          |
|                                                           |
|   johnm                                       01 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <algorithm>
#include "RGBControllerKeyNames.h"
#include "KeyboardLayoutManager.h"
#include "LogManager.h"
#include "RGBController_RoyalKludge.h"

typedef struct
{
    const char*     name;
    unsigned int    buffer_index;
} royal_kludge_led;

/*---------------------------------------------------------*\
| RK987 RGB per-key buffer indices                          |
|                                                           |
| Taken from the reverse engineered RK987RGB configuration  |
| (Rangoli project, keyboards/258a/configs/1ff.json).       |
\*---------------------------------------------------------*/
static const royal_kludge_led royal_kludge_rk987_leds[] =
{
    { KEY_EN_ESCAPE,                    0   },
    { KEY_EN_F1,                        12  },
    { KEY_EN_F2,                        18  },
    { KEY_EN_F3,                        24  },
    { KEY_EN_F4,                        30  },
    { KEY_EN_F5,                        36  },
    { KEY_EN_F6,                        42  },
    { KEY_EN_F7,                        48  },
    { KEY_EN_F8,                        54  },
    { KEY_EN_F9,                        60  },
    { KEY_EN_F10,                       66  },
    { KEY_EN_F11,                       72  },
    { KEY_EN_F12,                       78  },
    { KEY_EN_PRINT_SCREEN,              84  },
    { KEY_EN_SCROLL_LOCK,               90  },
    { KEY_EN_PAUSE_BREAK,               96  },
    { KEY_EN_BACK_TICK,                 1   },
    { KEY_EN_1,                         7   },
    { KEY_EN_2,                         13  },
    { KEY_EN_3,                         19  },
    { KEY_EN_4,                         25  },
    { KEY_EN_5,                         31  },
    { KEY_EN_6,                         37  },
    { KEY_EN_7,                         43  },
    { KEY_EN_8,                         49  },
    { KEY_EN_9,                         55  },
    { KEY_EN_0,                         61  },
    { KEY_EN_MINUS,                     67  },
    { KEY_EN_EQUALS,                    73  },
    { KEY_EN_BACKSPACE,                 79  },
    { KEY_EN_INSERT,                    85  },
    { KEY_EN_HOME,                      91  },
    { KEY_EN_PAGE_UP,                   97  },
    { KEY_EN_TAB,                       2   },
    { KEY_EN_Q,                         8   },
    { KEY_EN_W,                         14  },
    { KEY_EN_E,                         20  },
    { KEY_EN_R,                         26  },
    { KEY_EN_T,                         32  },
    { KEY_EN_Y,                         38  },
    { KEY_EN_U,                         44  },
    { KEY_EN_I,                         50  },
    { KEY_EN_O,                         56  },
    { KEY_EN_P,                         62  },
    { KEY_EN_LEFT_BRACKET,              68  },
    { KEY_EN_RIGHT_BRACKET,             74  },
    { KEY_EN_ANSI_BACK_SLASH,           80  },
    { KEY_EN_DELETE,                    86  },
    { KEY_EN_END,                       92  },
    { KEY_EN_PAGE_DOWN,                 98  },
    { KEY_EN_CAPS_LOCK,                 3   },
    { KEY_EN_A,                         9   },
    { KEY_EN_S,                         15  },
    { KEY_EN_D,                         21  },
    { KEY_EN_F,                         27  },
    { KEY_EN_G,                         33  },
    { KEY_EN_H,                         39  },
    { KEY_EN_J,                         45  },
    { KEY_EN_K,                         51  },
    { KEY_EN_L,                         57  },
    { KEY_EN_SEMICOLON,                 63  },
    { KEY_EN_QUOTE,                     69  },
    { KEY_EN_LEFT_SHIFT,                4   },
    { KEY_EN_Z,                         10  },
    { KEY_EN_X,                         16  },
    { KEY_EN_C,                         22  },
    { KEY_EN_V,                         28  },
    { KEY_EN_B,                         34  },
    { KEY_EN_N,                         40  },
    { KEY_EN_M,                         46  },
    { KEY_EN_COMMA,                     52  },
    { KEY_EN_PERIOD,                    58  },
    { KEY_EN_FORWARD_SLASH,             64  },
    { KEY_EN_RIGHT_SHIFT,               82  },
    { KEY_EN_UP_ARROW,                  94  },
    { KEY_EN_LEFT_CONTROL,              5   },
    { KEY_EN_LEFT_WINDOWS,              11  },
    { KEY_EN_LEFT_ALT,                  17  },
    { KEY_EN_SPACE,                     35  },
    { KEY_EN_RIGHT_ALT,                 53  },
    { KEY_EN_RIGHT_FUNCTION,            59  },
    { KEY_EN_MENU,                      65  },
    { KEY_EN_RIGHT_CONTROL,             83  },
    { KEY_EN_LEFT_ARROW,                89  },
    { KEY_EN_DOWN_ARROW,                95  },
    { KEY_EN_RIGHT_ARROW,               101 },
    { KEY_EN_ANSI_ENTER,                81  },
};

#define ROYAL_KLUDGE_INVALID_INDEX      0xFFFFFFFF
#define ROYAL_KLUDGE_LED_COUNT          (sizeof(royal_kludge_rk987_leds) / sizeof(royal_kludge_rk987_leds[0]))

static unsigned int GetBufferIndex(const std::string& name)
{
    for(unsigned int led_idx = 0; led_idx < ROYAL_KLUDGE_LED_COUNT; led_idx++)
    {
        if(name == royal_kludge_rk987_leds[led_idx].name)
        {
            return(royal_kludge_rk987_leds[led_idx].buffer_index);
        }
    }

    return(ROYAL_KLUDGE_INVALID_INDEX);
}

static RGBColor HueToRGB(unsigned int hue)
{
    unsigned int region     = hue / 60;
    unsigned int remainder  = ((hue % 60) * 255) / 60;
    unsigned int inverse    = 255 - remainder;

    switch(region)
    {
        case 0:
            return(ToRGBColor(255, remainder, 0));

        case 1:
            return(ToRGBColor(inverse, 255, 0));

        case 2:
            return(ToRGBColor(0, 255, remainder));

        case 3:
            return(ToRGBColor(0, inverse, 255));

        case 4:
            return(ToRGBColor(remainder, 0, 255));

        default:
            return(ToRGBColor(255, 0, inverse));
    }
}

typedef struct
{
    const char*     name;
    int             value;
    bool            has_color;
} royal_kludge_effect;

static const royal_kludge_effect royal_kludge_effects[] =
{
    { "Ripples Shining",            ROYAL_KLUDGE_MODE_RIPPLES_SHINING,       true  },
    { "Rotating Windmill",          ROYAL_KLUDGE_MODE_ROTATING_WINDMILL,     true  },
    { "Sine Wave",                  ROYAL_KLUDGE_MODE_SINE_WAVE,             true  },
    { "Stars Twinkle",              ROYAL_KLUDGE_MODE_STARS_TWINKLE,         true  },
    { "Layer Upon Layer",           ROYAL_KLUDGE_MODE_LAYER_UPON_LAYER,      true  },
    { "Rich And Honored",           ROYAL_KLUDGE_MODE_RICH_AND_HONORED,      true  },
    { "Marquee Effect",             ROYAL_KLUDGE_MODE_MARQUEE_EFFECT,        true  },
    { "Rotating Storm",             ROYAL_KLUDGE_MODE_ROTATING_STORM,        true  },
    { "Serpentine Horse Race",      ROYAL_KLUDGE_MODE_SERPENTINE_HORSE_RACE, true  },
    { "Retro Snake",                ROYAL_KLUDGE_MODE_RETRO_SNAKE,           true  },
    { "Diagonal Transformation",    ROYAL_KLUDGE_MODE_DIAGONAL_TRANSFORM,    true  },
    { "Ambilight",                  ROYAL_KLUDGE_MODE_AMBILIGHT,             true  },
    { "Streamer",                   ROYAL_KLUDGE_MODE_STREAMER,              true  },
    { "Neon",                       ROYAL_KLUDGE_MODE_NEON,                  false },
    { "Shadow Disappear",           ROYAL_KLUDGE_MODE_SHADOW_DISAPPEAR,      true  },
    { "Flash Away",                 ROYAL_KLUDGE_MODE_FLASH_AWAY,            true  },
};

/**------------------------------------------------------------------*\
    @name Royal Kludge RK987
    @category Keyboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectRoyalKludgeKeyboard
    @comment RK987RGB 2.4G N (258A:01FF)
\*-------------------------------------------------------------------*/

RGBController_RoyalKludge::RGBController_RoyalKludge(RoyalKludgeController* controller_ptr)
{
    controller              = controller_ptr;

    name                    = controller->GetNameString();
    vendor                  = "Royal Kludge";
    type                    = DEVICE_TYPE_KEYBOARD;
    description             = "Royal Kludge RK987 Keyboard";
    location                = controller->GetDeviceLocation();
    serial                  = controller->GetSerialString();

    /*---------------------------------------------------------*\
    | Direct mode                                               |
    \*---------------------------------------------------------*/
    mode Direct;
    Direct.name             = "Direct";
    Direct.value            = ROYAL_KLUDGE_DIRECT_MODE_VALUE;
    Direct.flags            = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode       = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    /*---------------------------------------------------------*\
    | Static Rainbow mode                                       |
    \*---------------------------------------------------------*/
    mode StaticRainbow;
    StaticRainbow.name             = "Static Rainbow";
    StaticRainbow.value            = ROYAL_KLUDGE_RAINBOW_MODE_VALUE;
    StaticRainbow.flags            = MODE_FLAG_HAS_BRIGHTNESS;
    StaticRainbow.color_mode       = MODE_COLORS_NONE;
    StaticRainbow.brightness_min   = 0;
    StaticRainbow.brightness_max   = 5;
    StaticRainbow.brightness       = 5;
    modes.push_back(StaticRainbow);

    /*---------------------------------------------------------*\
    | Static mode                                               |
    \*---------------------------------------------------------*/
    mode Static;
    Static.name             = "Static";
    Static.value            = ROYAL_KLUDGE_MODE_STATIC;
    Static.flags            = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Static.color_mode       = MODE_COLORS_MODE_SPECIFIC;
    Static.colors_min       = 1;
    Static.colors_max       = 1;
    Static.colors.push_back(ToRGBColor(255, 255, 255));
    Static.brightness_min   = 0;
    Static.brightness_max   = 5;
    Static.brightness       = 5;
    modes.push_back(Static);

    /*---------------------------------------------------------*\
    | Breathing mode                                            |
    \*---------------------------------------------------------*/
    mode Breathing;
    Breathing.name              = "Breathing";
    Breathing.value             = ROYAL_KLUDGE_MODE_BREATHING;
    Breathing.flags             = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Breathing.color_mode        = MODE_COLORS_MODE_SPECIFIC;
    Breathing.colors_min        = 1;
    Breathing.colors_max        = 1;
    Breathing.colors.push_back(ToRGBColor(255, 255, 255));
    Breathing.speed_min         = 0;
    Breathing.speed_max         = 4;
    Breathing.speed             = 2;
    Breathing.brightness_min    = 0;
    Breathing.brightness_max    = 5;
    Breathing.brightness        = 5;
    modes.push_back(Breathing);

    /*---------------------------------------------------------*\
    | Neon Stream mode                                          |
    \*---------------------------------------------------------*/
    mode NeonStream;
    NeonStream.name             = "Neon Stream";
    NeonStream.value            = ROYAL_KLUDGE_MODE_NEON_STREAM;
    NeonStream.flags            = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    NeonStream.color_mode       = MODE_COLORS_RANDOM;
    NeonStream.speed_min        = 0;
    NeonStream.speed_max        = 4;
    NeonStream.speed            = 2;
    NeonStream.brightness_min   = 0;
    NeonStream.brightness_max   = 5;
    NeonStream.brightness       = 5;
    modes.push_back(NeonStream);

    /*---------------------------------------------------------*\
    | Rainbow Roulette mode                                     |
    \*---------------------------------------------------------*/
    mode RainbowRoulette;
    RainbowRoulette.name           = "Rainbow Roulette";
    RainbowRoulette.value          = ROYAL_KLUDGE_MODE_RAINBOW_ROULETTE;
    RainbowRoulette.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    RainbowRoulette.color_mode     = MODE_COLORS_RANDOM;
    RainbowRoulette.speed_min      = 0;
    RainbowRoulette.speed_max      = 4;
    RainbowRoulette.speed          = 2;
    RainbowRoulette.brightness_min = 0;
    RainbowRoulette.brightness_max = 5;
    RainbowRoulette.brightness     = 5;
    modes.push_back(RainbowRoulette);

    /*---------------------------------------------------------*\
    | Remaining hardware light effects                          |
    \*---------------------------------------------------------*/
    for(unsigned int effect_idx = 0; effect_idx < (sizeof(royal_kludge_effects) / sizeof(royal_kludge_effects[0])); effect_idx++)
    {
        const royal_kludge_effect* effect = &royal_kludge_effects[effect_idx];

        mode new_mode;
        new_mode.name           = effect->name;
        new_mode.value          = effect->value;
        new_mode.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;

        if(effect->has_color)
        {
            new_mode.flags     |= MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
            new_mode.color_mode = MODE_COLORS_MODE_SPECIFIC;
            new_mode.colors_min = 1;
            new_mode.colors_max = 1;
            new_mode.colors.push_back(ToRGBColor(255, 255, 255));
        }
        else
        {
            new_mode.color_mode = MODE_COLORS_NONE;
        }

        new_mode.speed_min      = 0;
        new_mode.speed_max      = 4;
        new_mode.speed          = 2;
        new_mode.brightness_min = 0;
        new_mode.brightness_max = 5;
        new_mode.brightness     = 5;

        modes.push_back(new_mode);
    }

    SetupZones();
}

RGBController_RoyalKludge::~RGBController_RoyalKludge()
{
    Shutdown();

    delete controller;
}

void RGBController_RoyalKludge::SetupZones()
{
    KeyboardLayoutManager new_kb(KEYBOARD_LAYOUT_ANSI_QWERTY, KEYBOARD_SIZE_TKL);

    zone new_zone;

    new_zone.name       = ZONE_EN_KEYBOARD;
    new_zone.type       = ZONE_TYPE_MATRIX;
    new_zone.leds_count = new_kb.GetKeyCount();
    new_zone.leds_min   = new_zone.leds_count;
    new_zone.leds_max   = new_zone.leds_count;
    new_zone.matrix_map = new_kb.GetKeyMap(KEYBOARD_MAP_FILL_TYPE_COUNT);

    zones.push_back(new_zone);

    led_columns.resize(new_zone.leds_count);

    for(unsigned int pos = 0; pos < new_zone.matrix_map.map.size(); pos++)
    {
        unsigned int led_idx = new_zone.matrix_map.map[pos];

        if(led_idx < new_zone.leds_count)
        {
            led_columns[led_idx] = pos % new_zone.matrix_map.width;
        }
    }

    for(unsigned int led_idx = 0; led_idx < new_zone.leds_count; led_idx++)
    {
        led new_led;

        new_led.name    = new_kb.GetKeyNameAt(led_idx);
        new_led.value   = GetBufferIndex(new_led.name);

        if(new_led.value == ROYAL_KLUDGE_INVALID_INDEX)
        {
            LOG_WARNING("[%s] No RK987 LED buffer index for key \"%s\"", name.c_str(), new_led.name.c_str());
        }

        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_RoyalKludge::DeviceUpdateLEDs()
{
    if(modes[active_mode].value == ROYAL_KLUDGE_RAINBOW_MODE_VALUE)
    {
        DeviceUpdateRainbow();
        return;
    }

    std::vector<unsigned int>   led_indices;
    std::vector<RGBColor>       led_colors;

    size_t color_count = std::min(leds.size(), colors.size());

    for(size_t led_idx = 0; led_idx < color_count; led_idx++)
    {
        if(leds[led_idx].value == ROYAL_KLUDGE_INVALID_INDEX)
        {
            continue;
        }

        led_indices.push_back(leds[led_idx].value);
        led_colors.push_back(colors[led_idx]);
    }

    controller->SetLEDsDirect(led_indices, led_colors);
}

void RGBController_RoyalKludge::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RoyalKludge::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_RoyalKludge::DeviceUpdateMode()
{
    mode& current_mode = modes[active_mode];

    if((current_mode.value == ROYAL_KLUDGE_DIRECT_MODE_VALUE)
     ||(current_mode.value == ROYAL_KLUDGE_RAINBOW_MODE_VALUE))
    {
        DeviceUpdateLEDs();
        return;
    }

    bool        random_color    = (current_mode.color_mode == MODE_COLORS_RANDOM);
    RGBColor    color           = current_mode.colors.empty() ? 0x00000000 : current_mode.colors[0];
    unsigned int speed          = current_mode.speed + 1;

    if(current_mode.speed_max == 0)
    {
        speed = 5;
    }

    controller->SetMode(
        (unsigned char)current_mode.value,
        (unsigned char)speed,
        (unsigned char)current_mode.brightness,
        color,
        random_color);
}

void RGBController_RoyalKludge::DeviceUpdateRainbow()
{
    std::vector<unsigned int>   led_indices;
    std::vector<RGBColor>       led_colors;

    unsigned int matrix_width       = zones[0].matrix_map.width;
    unsigned int brightness         = modes[active_mode].brightness;
    unsigned int brightness_max     = modes[active_mode].brightness_max;
    unsigned int brightness_scale   = 255;

    if(brightness_max > 0)
    {
        brightness_scale = (brightness * 255) / brightness_max;
    }

    for(unsigned int led_idx = 0; led_idx < leds.size(); led_idx++)
    {
        if(leds[led_idx].value == ROYAL_KLUDGE_INVALID_INDEX)
        {
            continue;
        }

        unsigned int hue = 0;

        if(matrix_width > 0)
        {
            hue = (led_columns[led_idx] * 360) / matrix_width;
        }
        else
        {
            hue = (led_idx * 360) / leds.size();
        }

        RGBColor color = HueToRGB(hue);

        unsigned char red   = (RGBGetRValue(color) * brightness_scale) / 255;
        unsigned char green = (RGBGetGValue(color) * brightness_scale) / 255;
        unsigned char blue  = (RGBGetBValue(color) * brightness_scale) / 255;

        led_indices.push_back(leds[led_idx].value);
        led_colors.push_back(ToRGBColor(red, green, blue));
    }

    controller->SetLEDsDirect(led_indices, led_colors);
}
