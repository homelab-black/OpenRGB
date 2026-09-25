/*---------------------------------------------------------*\
| RGBController_ChilkeyKeyboard.cpp                         |
|                                                           |
|   RGBController for Chilkey Slice75 HE keyboard           |
|                                                           |
|   Aaron Coyco                                 28 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBControllerKeyNames.h"
#include "RGBController_ChilkeyKeyboard.h"

#define NA  0xFFFFFFFF

/*-----------------------------------------------------*\
| Physical matrix map for the Slice75 HE (ANSI), a 75%   |
| layout with 80 keys, derived from the validated key    |
| positions used by the Chilkey Slice75 HE SignalRGB      |
| community plugin                                        |
\*-----------------------------------------------------*/
#define CHILKEY_SLICE75_MATRIX_WIDTH   35
#define CHILKEY_SLICE75_MATRIX_HEIGHT  6

static unsigned int matrix_map[CHILKEY_SLICE75_MATRIX_HEIGHT][CHILKEY_SLICE75_MATRIX_WIDTH] =
{
    /* Function row: Esc, F1-F12, Delete, Insert                             */
    {  0, NA, NA,  1, NA,  2, NA,  3, NA,  4, NA,  5, NA,  6, NA,  7, NA,  8, NA, NA,  9, NA, 10, NA, 11, NA, NA, NA, 12, NA, 13, NA, NA, NA, 14 },

    /* Number row: ` 1 2 3 4 5 6 7 8 9 0 - = Backspace PageUp                */
    { 15, NA, 16, NA, 17, NA, 18, NA, 19, NA, 20, NA, 21, NA, 22, NA, 23, NA, 24, NA, 25, NA, 26, NA, 27, NA, NA, 28, NA, NA, NA, 29, NA, NA, NA },

    /* QWERTY row: Tab Q W E R T Y U I O P [ ] \ PageDown                    */
    { NA, 30, NA, 31, NA, 32, NA, 33, NA, 34, NA, 35, NA, 36, NA, 37, NA, 38, NA, 39, NA, 40, NA, 41, NA, 42, NA, 43, NA, NA, NA, 44, NA, NA, NA },

    /* Home row: CapsLock A S D F G H J K L ; ' Enter                        */
    { NA, 45, NA, NA, 46, NA, 47, NA, 48, NA, 49, NA, 50, NA, 51, NA, 52, NA, 53, NA, 54, NA, 55, NA, 56, NA, NA, 57, NA, NA, NA, NA, NA, NA, NA },

    /* Bottom alpha row: LShift Z X C V B N M , . / RShift Up                */
    { NA, 58, NA, NA, 59, NA, 60, NA, 61, NA, 62, NA, 63, NA, 64, NA, 65, NA, 66, NA, 67, NA, 68, NA, NA, NA, 69, NA, NA, NA, NA, 70, NA, NA, NA },

    /* Modifiers/arrows: LCtrl LWin LAlt Space Fn RCtrl Left Down Right      */
    { NA, 71, NA, 72, NA, 73, NA, NA, NA, NA, NA, NA, NA, NA, 74, NA, NA, NA, NA, NA, NA, NA, 75, NA, NA, 76, NA, NA, NA, 77, NA, 78, NA, NA, 79 }
};

static const char* led_names[] =
{
    /* Function row */
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

    /* Number row */
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
    KEY_EN_PAGE_UP,

    /* QWERTY row */
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
    KEY_EN_PAGE_DOWN,

    /* Home row */
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

    /* Bottom alpha row */
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
    KEY_EN_UP_ARROW,

    /* Modifiers/arrows row */
    KEY_EN_LEFT_CONTROL,
    KEY_EN_LEFT_WINDOWS,
    KEY_EN_LEFT_ALT,
    KEY_EN_SPACE,
    KEY_EN_RIGHT_FUNCTION,
    KEY_EN_RIGHT_CONTROL,
    KEY_EN_LEFT_ARROW,
    KEY_EN_DOWN_ARROW,
    KEY_EN_RIGHT_ARROW
};

/*-----------------------------------------------------*\
| HID keyboard usages accepted by the firmware's KRGB    |
| command, indexed the same as led_names above.  Fn uses |
| the controller-specific ID 0x01, confirmed by capturing |
| Chilkey's official Web Driver while it painted only Fn  |
\*-----------------------------------------------------*/
static const unsigned char led_key_ids[] =
{
    /* Function row */
    0x29, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x4C, 0x49,

    /* Number row */
    0x35, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x2D, 0x2E, 0x2A, 0x4B,

    /* QWERTY row */
    0x2B, 0x14, 0x1A, 0x08, 0x15, 0x17, 0x1C, 0x18, 0x0C, 0x12, 0x13, 0x2F, 0x30, 0x31, 0x4E,

    /* Home row */
    0x39, 0x04, 0x16, 0x07, 0x09, 0x0A, 0x0B, 0x0D, 0x0E, 0x0F, 0x33, 0x34, 0x28,

    /* Bottom alpha row */
    0xE1, 0x1D, 0x1B, 0x06, 0x19, 0x05, 0x11, 0x10, 0x36, 0x37, 0x38, 0xE5, 0x52,

    /* Modifiers/arrows row */
    0xE0, 0xE3, 0xE2, 0x2C, 0x01, 0xE4, 0x50, 0x51, 0x4F
};

#define CHILKEY_SLICE75_LED_COUNT  (sizeof(led_names) / sizeof(led_names[0]))

/**------------------------------------------------------------------*\
    @name Chilkey Slice75 HE
    @category Keyboard
    @type HID
    @save :x:
    @direct :white_check_mark:
    @effects :x:
    @detectors DetectChilkeyKeyboardControllers
    @comment Only the ANSI Slice75 HE (VID 0x1CA3, PID 0x0701) has been
             validated.  The device is switched to its Custom Per-Key
             mode on initialization.
\*-------------------------------------------------------------------*/

RGBController_ChilkeyKeyboard::RGBController_ChilkeyKeyboard(ChilkeyKeyboardController* controller_ptr)
{
    controller          = controller_ptr;

    name                = controller->GetNameString();
    vendor              = "Chilkey";
    type                = DEVICE_TYPE_KEYBOARD;
    description         = "Chilkey Slice75 HE Keyboard";
    location            = controller->GetDeviceLocation();
    serial              = controller->GetSerialString();

    mode Direct;
    Direct.name         = "Direct";
    Direct.value        = 0;
    Direct.flags        = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode   = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();

    controller->Initialize();
}

RGBController_ChilkeyKeyboard::~RGBController_ChilkeyKeyboard()
{
    Shutdown();

    delete controller;
}

void RGBController_ChilkeyKeyboard::SetupZones()
{
    zone keyboard_zone;
    keyboard_zone.name              = ZONE_EN_KEYBOARD;
    keyboard_zone.type              = ZONE_TYPE_MATRIX;
    keyboard_zone.leds_min          = CHILKEY_SLICE75_LED_COUNT;
    keyboard_zone.leds_max          = CHILKEY_SLICE75_LED_COUNT;
    keyboard_zone.leds_count        = CHILKEY_SLICE75_LED_COUNT;
    keyboard_zone.matrix_map.Set(CHILKEY_SLICE75_MATRIX_HEIGHT, CHILKEY_SLICE75_MATRIX_WIDTH, (unsigned int*)&matrix_map);

    zones.push_back(keyboard_zone);

    for(unsigned int led_idx = 0; led_idx < CHILKEY_SLICE75_LED_COUNT; led_idx++)
    {
        led new_led;
        new_led.name = led_names[led_idx];
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_ChilkeyKeyboard::DeviceUpdateLEDs()
{
    std::vector<unsigned char> key_ids(led_key_ids, led_key_ids + CHILKEY_SLICE75_LED_COUNT);

    controller->SetColors(colors, key_ids);
}

void RGBController_ChilkeyKeyboard::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_ChilkeyKeyboard::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_ChilkeyKeyboard::DeviceUpdateMode()
{
}
