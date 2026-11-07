/*---------------------------------------------------------*\
| RGBController_HyperXAlloyRise.cpp                         |
|                                                           |
|   RGBController for HyperX Alloy Rise keyboard            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "RGBControllerKeyNames.h"
#include "RGBController_HyperXAlloyRise.h"

using namespace std::chrono_literals;

/*---------------------------------------------------------*\
| 0xFFFFFFFF indicates an unused entry in the matrix        |
\*---------------------------------------------------------*/
#define NA  0xFFFFFFFF

/*---------------------------------------------------------*\
| 6 rows x 22 columns. Column positions were measured by    |
| capturing the keyboard while NGENUITY ran a left-to-right |
| sweep effect. Layout: ISO, Spanish.                       |
\*---------------------------------------------------------*/
static unsigned int matrix_map[6][22] =
{
    {   0,  NA,   1,   2,   3,   4,  NA,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  NA },
    {  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  NA,  32,  33,  34,  35,  36,  37,  38,  39 },
    {  40,  NA,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  73,  53,  54,  55,  56,  57,  58,  59 },
    {  60,  NA,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  NA,  NA,  NA,  NA,  74,  75,  76,  NA },
    {  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  NA,  NA,  89,  NA,  90,  NA,  91,  92,  93,  94 },
    {  95,  96,  97,  NA,  98,  NA,  99,  NA,  NA, 100, 101, 102,  NA, 103, 104, 105, 106, 107,  NA, 108, 109,  NA }
};

/*---------------------------------------------------------*\
| Key LED names, in the order the keyboard reports them     |
| (row by row, left to right).                              |
\*---------------------------------------------------------*/
static const char* led_names[] =
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
    KEY_EN_SCROLL_LOCK,
    KEY_EN_PAUSE_BREAK,
    KEY_EN_MEDIA_PREVIOUS,
    KEY_EN_MEDIA_PLAY_PAUSE,
    KEY_EN_MEDIA_NEXT,
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
    KEY_EN_INSERT,
    KEY_EN_HOME,
    KEY_EN_PAGE_UP,
    KEY_EN_NUMPAD_LOCK,
    KEY_EN_NUMPAD_DIVIDE,
    KEY_EN_NUMPAD_TIMES,
    KEY_EN_NUMPAD_MINUS,
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
    KEY_EN_DELETE,
    KEY_EN_END,
    KEY_EN_PAGE_DOWN,
    KEY_EN_NUMPAD_7,
    KEY_EN_NUMPAD_8,
    KEY_EN_NUMPAD_9,
    KEY_EN_NUMPAD_PLUS,
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
    KEY_EN_POUND,
    KEY_EN_ISO_ENTER,
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
    KEY_EN_FORWARD_SLASH,
    KEY_EN_RIGHT_SHIFT,
    KEY_EN_UP_ARROW,
    KEY_EN_NUMPAD_1,
    KEY_EN_NUMPAD_2,
    KEY_EN_NUMPAD_3,
    KEY_EN_NUMPAD_ENTER,
    KEY_EN_LEFT_CONTROL,
    KEY_EN_LEFT_WINDOWS,
    KEY_EN_LEFT_ALT,
    "Key: Space (Left)",
    KEY_EN_SPACE,
    "Key: Space (Right)",
    KEY_EN_RIGHT_ALT,
    KEY_EN_RIGHT_FUNCTION,
    KEY_EN_MENU,
    KEY_EN_RIGHT_CONTROL,
    KEY_EN_LEFT_ARROW,
    KEY_EN_DOWN_ARROW,
    KEY_EN_RIGHT_ARROW,
    KEY_EN_NUMPAD_0,
    KEY_EN_NUMPAD_PERIOD
};

RGBController_HyperXAlloyRise::RGBController_HyperXAlloyRise(HyperXAlloyRiseController* controller_ptr)
{
    controller          = controller_ptr;

    name                = controller->GetNameString();
    vendor              = "HyperX";
    type                = DEVICE_TYPE_KEYBOARD;
    description         = "HyperX Alloy Rise Keyboard Device";
    location            = controller->GetDeviceLocation();
    serial              = controller->GetSerialString();

    mode Direct;
    Direct.name         = "Direct";
    Direct.value        = 0xFFFF;
    Direct.flags            = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Direct.brightness_min   = 0;
    Direct.brightness_max   = 100;
    Direct.brightness       = 100;
    Direct.color_mode       = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    SetupZones();

    last_update_time     = std::chrono::steady_clock::now();
    keepalive_thread_run = 1;
    keepalive_thread     = new std::thread(&RGBController_HyperXAlloyRise::KeepaliveThread, this);
}

RGBController_HyperXAlloyRise::~RGBController_HyperXAlloyRise()
{
    Shutdown();

    keepalive_thread_run = 0;
    keepalive_thread->join();
    delete keepalive_thread;

    delete controller;
}

void RGBController_HyperXAlloyRise::SetupZones()
{
    /*-----------------------------------------------------*\
    | Keyboard matrix                                       |
    \*-----------------------------------------------------*/
    zone keyboard_zone;
    keyboard_zone.name          = ZONE_EN_KEYBOARD;
    keyboard_zone.type          = ZONE_TYPE_MATRIX;
    keyboard_zone.leds_min      = HYPERX_ALLOY_RISE_KEY_LED_COUNT;
    keyboard_zone.leds_max      = HYPERX_ALLOY_RISE_KEY_LED_COUNT;
    keyboard_zone.leds_count    = HYPERX_ALLOY_RISE_KEY_LED_COUNT;
    keyboard_zone.matrix_map.Set(6, 22, (unsigned int *)&matrix_map);
    zones.push_back(keyboard_zone);

    for(unsigned int led_idx = 0; led_idx < HYPERX_ALLOY_RISE_KEY_LED_COUNT; led_idx++)
    {
        led new_led;
        new_led.name = led_names[led_idx];
        leds.push_back(new_led);
    }

    /*-----------------------------------------------------*\
    | Light bars on the left and right sides                |
    \*-----------------------------------------------------*/
    const char* bar_names[2] = { "Left Light Bar", "Right Light Bar" };

    for(unsigned int bar = 0; bar < 2; bar++)
    {
        zone bar_zone;
        bar_zone.name           = bar_names[bar];
        bar_zone.type           = ZONE_TYPE_LINEAR;
        bar_zone.leds_min       = HYPERX_ALLOY_RISE_BAR_LED_COUNT;
        bar_zone.leds_max       = HYPERX_ALLOY_RISE_BAR_LED_COUNT;
        bar_zone.leds_count     = HYPERX_ALLOY_RISE_BAR_LED_COUNT;
        zones.push_back(bar_zone);

        for(unsigned int seg = 0; seg < HYPERX_ALLOY_RISE_BAR_LED_COUNT; seg++)
        {
            led new_led;
            new_led.name = std::string(bar_names[bar]) + " " + std::to_string(seg + 1);
            leds.push_back(new_led);
        }
    }

    SetupColors();
}

void RGBController_HyperXAlloyRise::DeviceUpdateLEDs()
{
    /*-----------------------------------------------------*\
    | The keyboard has no brightness command: NGENUITY      |
    | scales the colors before sending them, so do the same |
    \*-----------------------------------------------------*/
    unsigned int brightness = modes[active_mode].brightness;

    if(brightness >= 100)
    {
        controller->SetLEDsDirect(colors);
    }
    else
    {
        std::vector<RGBColor> scaled_colors(colors.size());

        for(std::size_t led_idx = 0; led_idx < colors.size(); led_idx++)
        {
            scaled_colors[led_idx] = ToRGBColor((RGBGetRValue(colors[led_idx]) * brightness) / 100,
                                                (RGBGetGValue(colors[led_idx]) * brightness) / 100,
                                                (RGBGetBValue(colors[led_idx]) * brightness) / 100);
        }

        controller->SetLEDsDirect(scaled_colors);
    }

    last_update_time = std::chrono::steady_clock::now();
}

void RGBController_HyperXAlloyRise::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_HyperXAlloyRise::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_HyperXAlloyRise::DeviceUpdateMode()
{
    /*-----------------------------------------------------*\
    | Apply a brightness change right away                  |
    \*-----------------------------------------------------*/
    DeviceUpdateLEDs();
}

void RGBController_HyperXAlloyRise::KeepaliveThread()
{
    /*-----------------------------------------------------*\
    | NGENUITY keeps streaming a frame every ~42 ms, so     |
    | do the same while the Direct mode is active.          |
    \*-----------------------------------------------------*/
    while(keepalive_thread_run.load())
    {
        if(active_mode == 0)
        {
            if((std::chrono::steady_clock::now() - last_update_time) > std::chrono::milliseconds(40))
            {
                UpdateLEDsInternal();
            }
        }
        std::this_thread::sleep_for(10ms);
    }
}
