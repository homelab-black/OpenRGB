/*---------------------------------------------------------*\
| RGBController_GMK67.cpp                                   |
|                                                           |
|   RGBController for the GMK67 keyboard                    |
|                                                           |
|   Key positions, LED names and hardware LED indices are   |
|   ported from the SignalRGB "GMK68" plugin by Nollie.     |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "RGBControllerKeyNames.h"
#include "RGBController_GMK67.h"

#define NA  0xFFFFFFFF

/*-----------------------------------------------------------------*\
| GMK67 65% layout - 66 addressable keys on a 15 x 5 matrix.        |
|                                                                   |
| Each entry: { x, y, hardware LED index, key name }.               |
| The order of this table IS the OpenRGB LED order.                 |
\*-----------------------------------------------------------------*/
typedef struct
{
    unsigned int    x;
    unsigned int    y;
    unsigned char   hw_index;
    const char*     name;
} gmk67_key;

static const gmk67_key gmk67_keys[GMK67_LED_COUNT] =
{
    /*-- Row 0 -------------------------------------------------*/
    {  0, 0, 0x01, KEY_EN_BACK_TICK        },
    {  1, 0, 0x14, KEY_EN_1                },
    {  2, 0, 0x15, KEY_EN_2                },
    {  3, 0, 0x16, KEY_EN_3                },
    {  4, 0, 0x17, KEY_EN_4                },
    {  5, 0, 0x18, KEY_EN_5                },
    {  6, 0, 0x19, KEY_EN_6                },
    {  7, 0, 0x1a, KEY_EN_7                },
    {  8, 0, 0x1b, KEY_EN_8                },
    {  9, 0, 0x1c, KEY_EN_9                },
    { 10, 0, 0x1d, KEY_EN_0                },
    { 11, 0, 0x1e, KEY_EN_MINUS            },
    { 12, 0, 0x1f, KEY_EN_EQUALS           },
    { 13, 0, 0x67, KEY_EN_BACKSPACE        },

    /*-- Row 1 -------------------------------------------------*/
    {  0, 1, 0x25, KEY_EN_TAB              },
    {  1, 1, 0x26, KEY_EN_Q                },
    {  2, 1, 0x27, KEY_EN_W                },
    {  3, 1, 0x28, KEY_EN_E                },
    {  4, 1, 0x29, KEY_EN_R                },
    {  5, 1, 0x2a, KEY_EN_T                },
    {  6, 1, 0x2b, KEY_EN_Y                },
    {  7, 1, 0x2c, KEY_EN_U                },
    {  8, 1, 0x2d, KEY_EN_I                },
    {  9, 1, 0x2e, KEY_EN_O                },
    { 10, 1, 0x2f, KEY_EN_P                },
    { 11, 1, 0x30, KEY_EN_LEFT_BRACKET     },
    { 12, 1, 0x31, KEY_EN_RIGHT_BRACKET    },
    { 13, 1, 0x43, KEY_EN_ANSI_BACK_SLASH  },
    { 14, 1, 0x77, KEY_EN_DELETE           },

    /*-- Row 2 -------------------------------------------------*/
    {  0, 2, 0x37, KEY_EN_CAPS_LOCK        },
    {  1, 2, 0x38, KEY_EN_A                },
    {  2, 2, 0x39, KEY_EN_S                },
    {  3, 2, 0x3a, KEY_EN_D                },
    {  4, 2, 0x3b, KEY_EN_F                },
    {  5, 2, 0x3c, KEY_EN_G                },
    {  6, 2, 0x3d, KEY_EN_H                },
    {  7, 2, 0x3e, KEY_EN_J                },
    {  8, 2, 0x3f, KEY_EN_K                },
    {  9, 2, 0x40, KEY_EN_L                },
    { 10, 2, 0x41, KEY_EN_SEMICOLON        },
    { 11, 2, 0x42, KEY_EN_QUOTE            },
    { 12, 2, 0x55, KEY_EN_ANSI_ENTER       },
    { 13, 2, 0x76, KEY_EN_PAGE_UP          },

    /*-- Row 3 -------------------------------------------------*/
    {  0, 3, 0x49, KEY_EN_LEFT_SHIFT       },
    {  1, 3, 0x4a, KEY_EN_Z                },
    {  2, 3, 0x4b, KEY_EN_X                },
    {  3, 3, 0x4c, KEY_EN_C                },
    {  4, 3, 0x4d, KEY_EN_V                },
    {  5, 3, 0x4e, KEY_EN_B                },
    {  6, 3, 0x4f, KEY_EN_N                },
    {  7, 3, 0x50, KEY_EN_M                },
    {  8, 3, 0x51, KEY_EN_COMMA            },
    {  9, 3, 0x52, KEY_EN_PERIOD           },
    { 10, 3, 0x53, KEY_EN_FORWARD_SLASH    },
    { 11, 3, 0x54, KEY_EN_RIGHT_SHIFT      },
    { 12, 3, 0x65, KEY_EN_UP_ARROW         },
    { 13, 3, 0x79, KEY_EN_PAGE_DOWN        },

    /*-- Row 4 -------------------------------------------------*/
    {  0, 4, 0x5b, KEY_EN_LEFT_CONTROL     },
    {  1, 4, 0x5c, KEY_EN_LEFT_WINDOWS     },
    {  2, 4, 0x5d, KEY_EN_LEFT_ALT         },
    {  6, 4, 0x5e, KEY_EN_SPACE            },
    { 10, 4, 0x5f, KEY_EN_RIGHT_ALT        },
    { 11, 4, 0x60, KEY_EN_RIGHT_FUNCTION   },
    { 12, 4, 0x63, KEY_EN_LEFT_ARROW       },
    { 13, 4, 0x64, KEY_EN_DOWN_ARROW       },
    { 14, 4, 0x66, KEY_EN_RIGHT_ARROW      },
};

#define GMK67_MATRIX_WIDTH  15
#define GMK67_MATRIX_HEIGHT 5

/**------------------------------------------------------------------*\
    @name GMK67
    @category Keyboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectGMK67Controllers
    @comment The GMK67 shares its USB VID/PID (05AC:024F) with the
    Keychron K3 V2, so both devices may be listed. Use the "GMK67"
    entry for a GMK67 keyboard. Only Direct mode is supported.
\*-------------------------------------------------------------------*/

RGBController_GMK67::RGBController_GMK67(GMK67Controller* controller_ptr)
{
    controller  = controller_ptr;

    name        = "GMK67";
    vendor      = "GMK";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "GMK67 Keyboard";
    location    = controller->GetDeviceLocation();
    serial      = controller->GetSerialString();

    mode Direct;
    Direct.name         = "Direct";
    Direct.value        = 0;
    Direct.flags        = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode   = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_GMK67::~RGBController_GMK67()
{
    delete controller;
}

void RGBController_GMK67::SetupZones()
{
    zone keyboard_zone;
    keyboard_zone.name              = ZONE_EN_KEYBOARD;
    keyboard_zone.type              = ZONE_TYPE_MATRIX;
    keyboard_zone.leds_min          = GMK67_LED_COUNT;
    keyboard_zone.leds_max          = GMK67_LED_COUNT;
    keyboard_zone.leds_count        = GMK67_LED_COUNT;

    keyboard_zone.matrix_map.width  = GMK67_MATRIX_WIDTH;
    keyboard_zone.matrix_map.height = GMK67_MATRIX_HEIGHT;
    keyboard_zone.matrix_map.map.resize(GMK67_MATRIX_WIDTH * GMK67_MATRIX_HEIGHT);

    for(unsigned int i = 0; i < (GMK67_MATRIX_WIDTH * GMK67_MATRIX_HEIGHT); i++)
    {
        keyboard_zone.matrix_map.map[i] = NA;
    }

    for(unsigned int led_idx = 0; led_idx < GMK67_LED_COUNT; led_idx++)
    {
        const gmk67_key& key = gmk67_keys[led_idx];

        led new_led;
        new_led.name  = key.name;
        new_led.value = key.hw_index;
        leds.push_back(new_led);

        keyboard_zone.matrix_map.map[key.y * GMK67_MATRIX_WIDTH + key.x] = led_idx;
    }

    zones.push_back(keyboard_zone);

    SetupColors();
}

void RGBController_GMK67::SendFrame()
{
    unsigned char color_stream[GMK67_COLOR_STREAM_SIZE];

    memset(color_stream, 0x00, sizeof(color_stream));

    for(unsigned int led_idx = 0; led_idx < GMK67_LED_COUNT; led_idx++)
    {
        RGBColor      color  = colors[led_idx];
        unsigned int  offset = led_idx * 4;

        color_stream[offset + 0] = gmk67_keys[led_idx].hw_index;
        color_stream[offset + 1] = RGBGetRValue(color);
        color_stream[offset + 2] = RGBGetGValue(color);
        color_stream[offset + 3] = RGBGetBValue(color);
    }

    controller->SendColors(color_stream, sizeof(color_stream));
}

void RGBController_GMK67::DeviceUpdateLEDs()
{
    SendFrame();
}

void RGBController_GMK67::DeviceUpdateZoneLEDs(int /*zone*/)
{
    SendFrame();
}

void RGBController_GMK67::DeviceUpdateSingleLED(int /*led*/)
{
    SendFrame();
}

void RGBController_GMK67::DeviceUpdateMode()
{
    SendFrame();
}
