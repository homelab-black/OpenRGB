/*---------------------------------------------------------*\
| RGBController_MachinelikeK500.cpp                         |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBControllerKeyNames.h"
#include "RGBController_MachinelikeK500.h"

#include "../SinowealthKeyboardController/SinowealthKeyboard_modes.h"

#define NA              0xFFFFFFFF

static unsigned int tkl_matrix_map[6][17] =
    { {   0,  NA,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,  11,  12,  13,  14, 15},
      {  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31, 32},
      {  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  NA,  46,  47, 48},
      {  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64, 65},
      {  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81, 82},
      {  83,  84,  85,  NA,  NA,  86,  NA,  NA,  NA,  87,  88,  89,  90,  91,  92,  93, 94}};


static const char *led_names_tkl[] =
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
    KEY_EN_PRINT_SCREEN,
    KEY_EN_NUMPAD_MINUS,
    KEY_EN_NUMPAD_PLUS,

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
    KEY_EN_QUOTE,
    KEY_EN_POUND,
    KEY_EN_BACKSPACE,
    KEY_EN_NUMPAD_LOCK,
    KEY_EN_NUMPAD_DIVIDE,
    KEY_EN_NUMPAD_TIMES,

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
    KEY_EN_SEMICOLON,
    KEY_EN_MINUS,
    KEY_EN_NUMPAD_7,
    KEY_EN_NUMPAD_8,
    KEY_EN_NUMPAD_9,

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
    KEY_ES_ENIE,
    KEY_EN_LEFT_BRACKET,
    KEY_EN_RIGHT_BRACKET,
    KEY_EN_ANSI_ENTER,
    KEY_EN_NUMPAD_4,
    KEY_EN_NUMPAD_5,
    KEY_EN_NUMPAD_6,

    KEY_EN_LEFT_SHIFT,
    KEY_EN_ISO_BACK_SLASH,
    KEY_EN_Z,
    KEY_EN_X,
    KEY_EN_C,
    KEY_EN_V,
    KEY_EN_B,
    KEY_EN_N,
    KEY_EN_M,
    KEY_EN_COMMA,
    KEY_EN_PERIOD,
    KEY_EN_MINUS,
    KEY_EN_RIGHT_SHIFT,
    KEY_EN_UP_ARROW,
    KEY_EN_NUMPAD_1,
    KEY_EN_NUMPAD_2,
    KEY_EN_NUMPAD_3,

    KEY_EN_LEFT_CONTROL,
    KEY_EN_LEFT_WINDOWS,
    KEY_EN_LEFT_ALT,
    KEY_EN_SPACE,
    KEY_EN_RIGHT_ALT,
    KEY_EN_RIGHT_CONTROL,
    KEY_EN_LEFT_FUNCTION,
    KEY_EN_LEFT_ARROW,
    KEY_EN_DOWN_ARROW,
    KEY_EN_RIGHT_ARROW,
    KEY_EN_NUMPAD_0,
    KEY_EN_NUMPAD_PERIOD
};


RGBController_MachinelikeK500::RGBController_MachinelikeK500(MachinelikeK500Controller* controller_ptr, std::function<void()> callback)
{
    controller        = controller_ptr;
    shutdown_callback = callback;

    name        = "Machenike K500";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "Keyboard Gaming";
    location    = controller->GetLocation();
    serial      = controller->GetSerialString();

    mode Direct;
    Direct.name       = "Direct";
    Direct.value      = MODE_PER_KEY;
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();
}

RGBController_MachinelikeK500::~RGBController_MachinelikeK500()
{
    delete controller;
}

void RGBController_MachinelikeK500::SetupZones()
{
    zone new_zone;

    new_zone.name                   = ZONE_EN_KEYBOARD;
    new_zone.type                   = ZONE_TYPE_MATRIX;
    new_zone.leds_min               = 95;
    new_zone.leds_max               = 95;
    new_zone.leds_count             = 95;
    new_zone.matrix_map.Set(6, 17, (unsigned int *)&tkl_matrix_map);

    zones.push_back(new_zone);

    for(unsigned int led_idx = 0; led_idx < 95; led_idx++)
    {
        led new_led;
        new_led.name = led_names_tkl[led_idx];
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_MachinelikeK500::ResizeZone(int /*zone*/, int /*new_size*/)
{
    /*---------------------------------------------------------*\
    | This device does not support resizing zones               |
    \*---------------------------------------------------------*/
}

void RGBController_MachinelikeK500::DeviceUpdateLEDs()
{
    controller->SetLEDsDirect(colors);
}

void RGBController_MachinelikeK500::UpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_MachinelikeK500::UpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_MachinelikeK500::DeviceUpdateMode()
{
    unsigned int brightness  = BRIGHTNESS_FULL;
    controller->SetMode(modes[active_mode].value, brightness, modes[active_mode].speed, modes[active_mode].color_mode);
}
