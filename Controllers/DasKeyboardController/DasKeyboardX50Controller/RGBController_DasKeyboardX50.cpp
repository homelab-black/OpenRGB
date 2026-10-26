/*---------------------------------------------------------*\
| RGBController_DasKeyboardX50.cpp                          |
|                                                           |
|   RGBController for the Das Keyboard X50 RGB             |
|                                                           |
|   Sergio Presa                                15 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBController_DasKeyboardX50.h"
#include "RGBControllerKeyNames.h"

#define NA 0xFFFFFFFF

static unsigned int x50_matrix[6][23] =
{
    {34,NA,23,15,7,12,0,5,69,74,58,46,51,40,NA,43,76,81,NA,86,90,95,NA},
    {35,30,24,16,8,13,1,6,70,75,61,54,47,44,NA,45,82,83,79,85,94,89,NA},
    {36,31,25,17,9,14,2,71,62,67,55,48,52,41,NA,78,80,77,106,88,98,92,NA},
    {37,32,26,18,22,10,3,72,63,64,56,49,42,NA,NA,NA,NA,NA,103,87,96,92,NA},
    {38,27,29,21,19,11,4,73,65,66,57,53,NA,NA,NA,NA,99,NA,100,84,93,91,NA},
    {39,33,28,NA,NA,20,NA,NA,68,60,59,50,NA,NA,NA,104,101,102,105,NA,97,91,NA}
};

static const char* x50_led_names[] =
{
    KEY_EN_F5,
    KEY_EN_6,
    KEY_EN_Y,
    KEY_EN_H,
    KEY_EN_N,
    KEY_EN_F6,
    KEY_EN_7,
    KEY_EN_F3,
    KEY_EN_4,
    KEY_EN_R,
    KEY_EN_G,
    KEY_EN_B,
    KEY_EN_F4,
    KEY_EN_5,
    KEY_EN_T,
    KEY_EN_F2,
    KEY_EN_3,
    KEY_EN_E,
    KEY_EN_D,
    KEY_EN_V,
    KEY_EN_SPACE,
    KEY_EN_C,
    KEY_EN_F,
    KEY_EN_F1,
    KEY_EN_2,
    KEY_EN_W,
    KEY_EN_S,
    KEY_EN_Z,
    KEY_EN_LEFT_ALT,
    KEY_EN_X,
    KEY_EN_1,
    KEY_EN_Q,
    KEY_EN_A,
    KEY_EN_LEFT_WINDOWS,
    KEY_EN_ESCAPE,
    KEY_EN_BACK_TICK,
    KEY_EN_TAB,
    KEY_EN_CAPS_LOCK,
    KEY_EN_LEFT_SHIFT,
    KEY_EN_LEFT_CONTROL,
    KEY_EN_F12,
    KEY_EN_ANSI_BACK_SLASH,
    KEY_EN_ANSI_ENTER,
    KEY_EN_PRINT_SCREEN,
    KEY_EN_BACKSPACE,
    KEY_EN_INSERT,
    KEY_EN_F10,
    KEY_EN_EQUALS,
    KEY_EN_LEFT_BRACKET,
    KEY_EN_QUOTE,
    KEY_EN_RIGHT_CONTROL,
    KEY_EN_F11,
    KEY_EN_RIGHT_BRACKET,
    KEY_EN_RIGHT_SHIFT,
    KEY_EN_MINUS,
    KEY_EN_P,
    KEY_EN_SEMICOLON,
    KEY_EN_FORWARD_SLASH,
    KEY_EN_F9,
    KEY_EN_MENU,
    KEY_EN_RIGHT_FUNCTION,
    KEY_EN_0,
    KEY_EN_I,
    KEY_EN_K,
    KEY_EN_L,
    KEY_EN_COMMA,
    KEY_EN_PERIOD,
    KEY_EN_O,
    KEY_EN_RIGHT_ALT,
    KEY_EN_F7,
    KEY_EN_8,
    KEY_EN_U,
    KEY_EN_J,
    KEY_EN_M,
    KEY_EN_F8,
    KEY_EN_9,
    KEY_EN_SCROLL_LOCK,
    KEY_EN_PAGE_DOWN,
    KEY_EN_DELETE,
    KEY_EN_NUMPAD_LOCK,
    KEY_EN_END,
    KEY_EN_PAUSE_BREAK,
    KEY_EN_HOME,
    KEY_EN_PAGE_UP,
    KEY_EN_NUMPAD_2,
    KEY_EN_NUMPAD_DIVIDE,
    KEY_EN_BRIGHTNESS,
    KEY_EN_NUMPAD_5,
    KEY_EN_NUMPAD_8,
    KEY_EN_NUMPAD_MINUS,
    KEY_EN_MEDIA_PLAY_PAUSE,
    KEY_EN_NUMPAD_ENTER,
    KEY_EN_NUMPAD_PLUS,
    KEY_EN_NUMPAD_3,
    KEY_EN_NUMPAD_TIMES,
    KEY_EN_MEDIA_NEXT,
    KEY_EN_NUMPAD_6,
    KEY_EN_NUMPAD_PERIOD,
    KEY_EN_NUMPAD_9,
    KEY_EN_UP_ARROW,
    KEY_EN_NUMPAD_1,
    KEY_EN_DOWN_ARROW,
    KEY_EN_RIGHT_ARROW,
    KEY_EN_NUMPAD_4,
    KEY_EN_LEFT_ARROW,
    KEY_EN_NUMPAD_0,
    KEY_EN_NUMPAD_7,
    "Volume Ring 0-90",
    "Volume Ring 90-180",
    "Volume Ring 180-270",
    "Volume Ring 270-360"
};

/**------------------------------------------------------------------*\
    @name Das Keyboard X50 RGB
    @category Keyboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectDasKeyboardX50
    @comment
\*-------------------------------------------------------------------*/

RGBController_DasKeyboardX50::RGBController_DasKeyboardX50(DasKeyboardX50Controller* controller_ptr)
    : controller(controller_ptr)
{
    name                    = "Das Keyboard X50 RGB";
    vendor                  = "Das Keyboard";
    type                    = DEVICE_TYPE_KEYBOARD;
    description             = "Das Keyboard X50 RGB Mechanical Keyboard";
    location                = controller->GetLocationString();
    serial                  = controller->GetSerialString();

    mode direct;

    direct.name           = "Direct";
    direct.value          = 0;
    direct.flags          = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    direct.color_mode     = MODE_COLORS_PER_LED;
    direct.brightness_min = 0;
    direct.brightness_max = 100;
    direct.brightness     = 100;
    modes.push_back(direct);
    SetupZones();
}

RGBController_DasKeyboardX50::~RGBController_DasKeyboardX50()
{
    Shutdown();
    delete controller;
}

void RGBController_DasKeyboardX50::SetupZones()
{
    zone keyboard;

    keyboard.name           = "Keyboard";
    keyboard.type           = ZONE_TYPE_MATRIX;
    keyboard.leds_count     = 107;
    keyboard.leds_min       = 107;
    keyboard.leds_max       = 107;
    keyboard.matrix_map.Set(6, 23, (unsigned int*)&x50_matrix);
    zones.push_back(keyboard);

    zone ring;

    ring.name               = "Volume Ring";
    ring.type               = ZONE_TYPE_LINEAR;
    ring.leds_count         = 4;
    ring.leds_min           = 4;
    ring.leds_max           = 4;
    zones.push_back(ring);

    for(unsigned int led_idx = 0; led_idx < controller->LedCount(); led_idx++)
    {
        led item;

        item.name = x50_led_names[led_idx];
        leds.push_back(item);
    }

    SetupColors();
}

void RGBController_DasKeyboardX50::DeviceUpdateLEDs()
{
    const unsigned int brightness = modes[active_mode].brightness;
    std::vector<RGBColor> scaled_colors;
    scaled_colors.reserve(colors.size());

    for(unsigned int color_idx = 0; color_idx < colors.size(); color_idx++)
    {
        RGBColor color             = colors[color_idx];
        const unsigned char red   = static_cast<unsigned char>((RGBGetRValue(color) * brightness + 50) / 100);
        const unsigned char green = static_cast<unsigned char>((RGBGetGValue(color) * brightness + 50) / 100);
        const unsigned char blue  = static_cast<unsigned char>((RGBGetBValue(color) * brightness + 50) / 100);
        scaled_colors.push_back(ToRGBColor(red, green, blue));
    }

    controller->SendColors(scaled_colors);
}

void RGBController_DasKeyboardX50::DeviceUpdateZoneLEDs(int)
{
    DeviceUpdateLEDs();
}

void RGBController_DasKeyboardX50::DeviceUpdateSingleLED(int)
{
    DeviceUpdateLEDs();
}

void RGBController_DasKeyboardX50::DeviceUpdateMode()
{
}
