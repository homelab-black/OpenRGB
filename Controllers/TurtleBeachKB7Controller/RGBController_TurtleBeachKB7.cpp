/*---------------------------------------------------------*\
| RGBController_TurtleBeachKB7.cpp                          |
|                                                           |
|   RGBController for Turtle Beach Command Series KB7       |
|                                                           |
|   Joe Peterson (joevpeterson@gmail.com)       04 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <string>
#include "RGBController_TurtleBeachKB7.h"
#include "RGBControllerKeyNames.h"

/*---------------------------------------------------------*\
| Maps each LED index to its row and column in the device's |
| planar frame buffer.  The rows here are the device's      |
| internal scan matrix, not the physical keyboard rows -    |
| keys that sit next to each other on the keyboard can      |
| land in different buffer rows.  The positions that are    |
| absent from this table are gaps in the matrix with no     |
| LED behind them.                                          |
|                                                           |
| Because the ordering does not follow the physical layout, |
| the zones are linear rather than carrying a matrix map.   |
| A matrix map could be added later once each key position  |
| has been identified on hardware.                          |
\*---------------------------------------------------------*/
struct kb7_led_position
{
    unsigned char row;
    unsigned char col;
};

static const kb7_led_position turtle_beach_kb7_led_positions[TURTLE_BEACH_KB7_LED_COUNT] =
{
    /*-----------------------------------------------------*\
    | Hardware keys, which sit above the function row        |
    \*-----------------------------------------------------*/
    {5, 5}, {5, 6}, {5, 7}, {5, 8}, {5, 9}, {5,10},

    /*-----------------------------------------------------*\
    | Escape and the function row                            |
    \*-----------------------------------------------------*/
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6},
    {0, 7}, {0, 8}, {0, 9}, {0,10}, {0,11}, {0,12},

    /*-----------------------------------------------------*\
    | Number row                                             |
    \*-----------------------------------------------------*/
    {0,14}, {0,15}, {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4},
    {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, {1,10}, {1,11},

    /*-----------------------------------------------------*\
    | QWERTY row                                             |
    \*-----------------------------------------------------*/
    {1,12}, {1,13}, {1,14}, {1,15}, {2, 0}, {2, 1}, {2, 2},
    {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 9},

    /*-----------------------------------------------------*\
    | Home row                                               |
    \*-----------------------------------------------------*/
    {2,10}, {2,11}, {2,12}, {2,13}, {2,14}, {2,15}, {3, 0},
    {3, 1}, {3, 2}, {3, 3}, {3, 4}, {3, 5}, {3, 7},

    /*-----------------------------------------------------*\
    | Shift row, ending with the up arrow                    |
    \*-----------------------------------------------------*/
    {3, 8}, {3,10}, {3,11}, {3,12}, {3,13}, {3,14}, {3,15},
    {4, 0}, {4, 1}, {4, 2}, {4, 3}, {4, 5}, {4, 6},

    /*-----------------------------------------------------*\
    | Bottom row and the remaining arrows                    |
    \*-----------------------------------------------------*/
    {4, 7}, {4, 8}, {4, 9}, {4,11}, {4,14}, {4,15}, {5, 0},
    {5, 1}, {5, 2}, {5, 3}, {5, 4},

    /*-----------------------------------------------------*\
    | Wrist rest glow                                        |
    \*-----------------------------------------------------*/
    {6, 0}, {6, 1}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {6, 6},
    {6, 7}, {6, 8}, {6, 9},
};

/**------------------------------------------------------------------*\
    @name Turtle Beach Command Series KB7
    @category Keyboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectTurtleBeachKB7Controllers
    @comment Only the raw per-key color stream is implemented.  The
             Command Display, the keyboard and media key collection,
             and the touchscreen's boot protocol mouse interface are
             not handled.  The device must be switched into host
             controlled mode by a feature report on a separate
             interface before it accepts color data.  LEDs are split
             into three linear zones - hardware keys, keyboard, and
             wrist rest - rather than a matrix, as the buffer ordering
             does not follow the physical key layout.
\*-------------------------------------------------------------------*/

RGBController_TurtleBeachKB7::RGBController_TurtleBeachKB7(TurtleBeachKB7Controller* controller_ptr)
{
    controller                  = controller_ptr;

    name                        = "Turtle Beach Command Series KB7";
    vendor                      = "Turtle Beach";
    type                        = DEVICE_TYPE_KEYBOARD;
    description                 = "Turtle Beach Command Series KB7 Keyboard";
    location                    = controller->GetLocation();
    serial                      = controller->GetSerialString();

    mode Direct;
    Direct.name                 = "Direct";
    Direct.value                = TURTLE_BEACH_KB7_MODE_DIRECT;
    Direct.flags                = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode           = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_TurtleBeachKB7::~RGBController_TurtleBeachKB7()
{
    Shutdown();

    delete controller;
}

void RGBController_TurtleBeachKB7::SetupZones()
{
    /*-----------------------------------------------------*\
    | Names for the hardware keys and the standard keys, in |
    | device LED order - the first six are the hardware key |
    | zone, the remaining ones are the keyboard zone.  This |
    | is a local array rather than a file scope one because |
    | the KEY_EN_ values are defined in another translation |
    | unit and are not safe to read during static init.     |
    \*-----------------------------------------------------*/
    const char* key_names[TURTLE_BEACH_KB7_NONWRIST_COUNT] =
    {
        /*-------------------------------------------------*\
        | Dedicated hardware keys above the function row.   |
        | Mode Select and Profile have no standard constant |
        | in RGBControllerKeyNames.h, so they are literals, |
        | as is RGB Off - it kills the lighting outright    |
        | rather than cycling levels like KEY_EN_BRIGHTNESS.|
        \*-------------------------------------------------*/
        KEY_EN_MEDIA_PREVIOUS, KEY_EN_MEDIA_NEXT, "Key: Mode", "Key: Profile",
        KEY_EN_MEDIA_MUTE,  "Key: RGB Off",

        KEY_EN_ESCAPE,      KEY_EN_F1,          KEY_EN_F2,          KEY_EN_F3,
        KEY_EN_F4,          KEY_EN_F5,          KEY_EN_F6,          KEY_EN_F7,
        KEY_EN_F8,          KEY_EN_F9,          KEY_EN_F10,         KEY_EN_F11,
        KEY_EN_F12,

        KEY_EN_BACK_TICK,   KEY_EN_1,           KEY_EN_2,           KEY_EN_3,
        KEY_EN_4,           KEY_EN_5,           KEY_EN_6,           KEY_EN_7,
        KEY_EN_8,           KEY_EN_9,           KEY_EN_0,           KEY_EN_MINUS,
        KEY_EN_EQUALS,      KEY_EN_BACKSPACE,

        KEY_EN_TAB,         KEY_EN_Q,           KEY_EN_W,           KEY_EN_E,
        KEY_EN_R,           KEY_EN_T,           KEY_EN_Y,           KEY_EN_U,
        KEY_EN_I,           KEY_EN_O,           KEY_EN_P,           KEY_EN_LEFT_BRACKET,
        KEY_EN_RIGHT_BRACKET, KEY_EN_ANSI_BACK_SLASH,

        KEY_EN_CAPS_LOCK,   KEY_EN_A,           KEY_EN_S,           KEY_EN_D,
        KEY_EN_F,           KEY_EN_G,           KEY_EN_H,           KEY_EN_J,
        KEY_EN_K,           KEY_EN_L,           KEY_EN_SEMICOLON,   KEY_EN_QUOTE,
        KEY_EN_ANSI_ENTER,

        KEY_EN_LEFT_SHIFT,  KEY_EN_Z,           KEY_EN_X,           KEY_EN_C,
        KEY_EN_V,           KEY_EN_B,           KEY_EN_N,           KEY_EN_M,
        KEY_EN_COMMA,       KEY_EN_PERIOD,      KEY_EN_FORWARD_SLASH, KEY_EN_RIGHT_SHIFT,
        KEY_EN_UP_ARROW,

        KEY_EN_LEFT_CONTROL, KEY_EN_LEFT_WINDOWS, KEY_EN_LEFT_ALT,  KEY_EN_SPACE,
        KEY_EN_RIGHT_ALT,   KEY_EN_RIGHT_FUNCTION, KEY_EN_MENU,     KEY_EN_RIGHT_CONTROL,
        KEY_EN_LEFT_ARROW,  KEY_EN_DOWN_ARROW,  KEY_EN_RIGHT_ARROW,
    };

    /*-----------------------------------------------------*\
    | The hardware keys, the main key area, and the wrist   |
    | rest glow are three physically separate lighting      |
    | elements, so each is exposed as its own zone.  Their  |
    | LEDs are contiguous and in this order in the tables.  |
    \*-----------------------------------------------------*/
    zone hwkey_zone;

    hwkey_zone.name             = "Hardware Keys";
    hwkey_zone.type             = ZONE_TYPE_LINEAR;
    hwkey_zone.leds_min         = TURTLE_BEACH_KB7_HWKEY_COUNT;
    hwkey_zone.leds_max         = TURTLE_BEACH_KB7_HWKEY_COUNT;
    hwkey_zone.leds_count       = TURTLE_BEACH_KB7_HWKEY_COUNT;

    zones.push_back(hwkey_zone);

    zone key_zone;

    key_zone.name               = ZONE_EN_KEYBOARD;
    key_zone.type               = ZONE_TYPE_LINEAR;
    key_zone.leds_min           = TURTLE_BEACH_KB7_KEY_COUNT;
    key_zone.leds_max           = TURTLE_BEACH_KB7_KEY_COUNT;
    key_zone.leds_count         = TURTLE_BEACH_KB7_KEY_COUNT;

    zones.push_back(key_zone);

    zone wrist_zone;

    wrist_zone.name             = "Wrist Rest";
    wrist_zone.type             = ZONE_TYPE_LINEAR;
    wrist_zone.leds_min         = TURTLE_BEACH_KB7_WRIST_COUNT;
    wrist_zone.leds_max         = TURTLE_BEACH_KB7_WRIST_COUNT;
    wrist_zone.leds_count       = TURTLE_BEACH_KB7_WRIST_COUNT;

    zones.push_back(wrist_zone);

    for(unsigned int led_idx = 0; led_idx < TURTLE_BEACH_KB7_NONWRIST_COUNT; led_idx++)
    {
        led new_led;

        new_led.name            = key_names[led_idx];
        new_led.value           = led_idx;

        leds.push_back(new_led);
    }

    for(unsigned int led_idx = 0; led_idx < TURTLE_BEACH_KB7_WRIST_COUNT; led_idx++)
    {
        led new_led;

        new_led.name            = "Wrist Rest " + std::to_string(led_idx + 1);
        new_led.value           = TURTLE_BEACH_KB7_NONWRIST_COUNT + led_idx;

        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_TurtleBeachKB7::DeviceUpdateLEDs()
{
    unsigned char led_data[TURTLE_BEACH_KB7_BUFFER_SIZE] = { 0 };

    /*-----------------------------------------------------*\
    | Buffer is planar: each row is 3 consecutive 16 byte   |
    | blocks (R, then G, then B), not RGB-interleaved       |
    | triplets - see TurtleBeachKB7Controller.h.            |
    \*-----------------------------------------------------*/
    for(unsigned int led_idx = 0; (led_idx < colors.size()) && (led_idx < TURTLE_BEACH_KB7_LED_COUNT); led_idx++)
    {
        unsigned int row_base = turtle_beach_kb7_led_positions[led_idx].row * (TURTLE_BEACH_KB7_ROW_BYTES);
        unsigned int col      = turtle_beach_kb7_led_positions[led_idx].col;

        led_data[row_base + col]                                       = RGBGetRValue(colors[led_idx]);
        led_data[row_base + TURTLE_BEACH_KB7_PLANE_WIDTH + col]        = RGBGetGValue(colors[led_idx]);
        led_data[row_base + (2 * TURTLE_BEACH_KB7_PLANE_WIDTH) + col]  = RGBGetBValue(colors[led_idx]);
    }

    controller->SendDirect(led_data, sizeof(led_data));
}

void RGBController_TurtleBeachKB7::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_TurtleBeachKB7::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_TurtleBeachKB7::DeviceUpdateMode()
{
    /*-----------------------------------------------------*\
    | Only Direct mode is implemented                       |
    \*-----------------------------------------------------*/
}
