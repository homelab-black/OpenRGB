/*---------------------------------------------------------*\
| RGBController_CosmicByteFireflyKeyboard.h                 |
|                                                           |
|   RGBcontroller for Cosmic Byte Firefly TKL Keyboard      |
|                                                           |
|                                                           |
|   Akankshya Das                               25 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "LogManager.h"
#include "RGBControllerKeyNames.h"
#include "RGBcontroller_CosmicByteFireflyKeyboard.h"

/*========================================================================*\
| 87-KEY TKL MATRIX MAPPING & LED NAME ARRAYS                              |
\*========================================================================*/

#define NA 0xFFFFFFFF

static unsigned int matrix_map_firefly[6][18] =
{
    {   0,  NA,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  NA },
    {  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  NA,  30,  31,  32 },
    {  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  NA,  47,  48,  49 },
    {  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  62,  NA,  NA,  NA,  NA },
    {  63,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  74,  NA,  NA,  75,  NA },
    {  76,  77,  78,  79,  79,  79,  79,  79,  80,  81,  82,  83,  NA,  NA,  NA,  84,  85,  86 }
};

static const char *led_names[] =
{
    // Row 1 (0 - 15)
    KEY_EN_ESCAPE, KEY_EN_F1, KEY_EN_F2, KEY_EN_F3, KEY_EN_F4, KEY_EN_F5, KEY_EN_F6, KEY_EN_F7, KEY_EN_F8, KEY_EN_F9, KEY_EN_F10, KEY_EN_F11, KEY_EN_F12,
    KEY_EN_PRINT_SCREEN, KEY_EN_SCROLL_LOCK, KEY_EN_PAUSE_BREAK,
    
    // Row 2 (16 - 32)
    KEY_EN_BACK_TICK, KEY_EN_1, KEY_EN_2, KEY_EN_3, KEY_EN_4, KEY_EN_5, KEY_EN_6, KEY_EN_7, KEY_EN_8, KEY_EN_9, KEY_EN_0, KEY_EN_MINUS, KEY_EN_EQUALS, KEY_EN_BACKSPACE,
    KEY_EN_INSERT, KEY_EN_HOME, KEY_EN_PAGE_UP,
    
    // Row 3 (33 - 49)
    KEY_EN_TAB, KEY_EN_Q, KEY_EN_W, KEY_EN_E, KEY_EN_R, KEY_EN_T, KEY_EN_Y, KEY_EN_U, KEY_EN_I, KEY_EN_O, KEY_EN_P, KEY_EN_LEFT_BRACKET, KEY_EN_RIGHT_BRACKET, KEY_EN_ANSI_BACK_SLASH,
    KEY_EN_DELETE, KEY_EN_END, KEY_EN_PAGE_DOWN,
    
    // Row 4 (50 - 62)
    KEY_EN_CAPS_LOCK, KEY_EN_A, KEY_EN_S, KEY_EN_D, KEY_EN_F, KEY_EN_G, KEY_EN_H, KEY_EN_J, KEY_EN_K, KEY_EN_L, KEY_EN_SEMICOLON, KEY_EN_QUOTE, KEY_EN_ANSI_ENTER,
    
    // Row 5 (63 - 75)
    KEY_EN_LEFT_SHIFT, KEY_EN_Z, KEY_EN_X, KEY_EN_C, KEY_EN_V, KEY_EN_B, KEY_EN_N, KEY_EN_M, KEY_EN_COMMA, KEY_EN_PERIOD, KEY_EN_FORWARD_SLASH, KEY_EN_RIGHT_SHIFT, KEY_EN_UP_ARROW,
    
    // Row 6 (76 - 86)
    KEY_EN_LEFT_CONTROL, KEY_EN_LEFT_WINDOWS, KEY_EN_LEFT_ALT, KEY_EN_SPACE, KEY_EN_RIGHT_ALT, KEY_EN_RIGHT_FUNCTION, KEY_EN_MENU, KEY_EN_RIGHT_CONTROL, KEY_EN_LEFT_ARROW, KEY_EN_DOWN_ARROW, KEY_EN_RIGHT_ARROW
};

RGBcontroller_CosmicByteFireflyKeyboard::RGBcontroller_CosmicByteFireflyKeyboard(CosmicByteFireflyKeyboardController* controller_ptr)
{
    controller  = controller_ptr;

    name        = controller->GetName();
    vendor      = "Firefly";
    type        = DEVICE_TYPE_KEYBOARD;
    description = "Firefly Custom 87-Key TKL RGB Keyboard";
    location    = controller->GetLocation();

    // =========================================================================
    // 1. Direct Mode (Per-Key Streaming Control)
    // =========================================================================
    mode direct_mode;
    direct_mode.name           = "Direct";
    direct_mode.value          = 0xFF; // Internal identifier for per-key streaming
    direct_mode.flags          = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    direct_mode.color_mode     = MODE_COLORS_PER_LED;
    direct_mode.brightness_min = 0x09;
    direct_mode.brightness_max = 0x3F;
    direct_mode.brightness     = 0x3F;
    modes.push_back(direct_mode);

    // =========================================================================
    // 2. Global Static Mode
    // =========================================================================
    mode static_mode;
    static_mode.name           = "Static";
    static_mode.value          = 0x00;
    static_mode.flags          = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    static_mode.color_mode     = MODE_COLORS_MODE_SPECIFIC;
    static_mode.colors.resize(1);
    static_mode.colors[0]      = ToRGBColor(255, 0, 0); // Default Red
    static_mode.brightness_min = 0x09;
    static_mode.brightness_max = 0x3F;
    static_mode.brightness     = 0x3F;
    modes.push_back(static_mode);

    // =========================================================================
    // 3. Hardware Animations Array 
    // =========================================================================
    const struct { const char* name; unsigned char id; bool has_direction; } hw_animations[] =
    {
        { "Breathe",      0x01, false },
        { "Fade",         0x02, false },
        { "Getting Off",  0x03, false },
        { "Little Stars", 0x04, false },
        { "Laser",        0x05, false },
        { "Wave",         0x06, true  },
        { "Neon",         0x07, false },
        { "Raindrop",     0x08, false },
        { "Ripple",       0x09, false },
        { "Wave 2",       0x0A, false },
        { "Swirl",        0x0B, false }
    };

    for(size_t i = 0; i < (sizeof(hw_animations) / sizeof(hw_animations[0])); i++)
    {
        mode new_mode;
        new_mode.name           = hw_animations[i].name;
        new_mode.value          = hw_animations[i].id;
        new_mode.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS;

        if(hw_animations[i].has_direction)
        {
            new_mode.flags     |= MODE_FLAG_HAS_DIRECTION_LR;
            new_mode.direction  = MODE_DIRECTION_RIGHT;
        }

        new_mode.color_mode     = MODE_COLORS_NONE;
        new_mode.speed_min      = 0x07;
        new_mode.speed_max      = 0x01;
        new_mode.speed          = 0x04;
        new_mode.brightness_min = 0x09;
        new_mode.brightness_max = 0x3F;
        new_mode.brightness     = 0x3F;

        modes.push_back(new_mode);
    }

    SetupZones();
}

RGBcontroller_CosmicByteFireflyKeyboard::~RGBcontroller_CosmicByteFireflyKeyboard()
{
    delete controller;
}

void RGBcontroller_CosmicByteFireflyKeyboard::SetupZones()
{
    // Full Matrix Setup for 87-key TKL Layout
    zone new_zone;
    new_zone.name       = ZONE_EN_KEYBOARD;
    new_zone.type       = ZONE_TYPE_MATRIX;
    new_zone.leds_min   = sizeof(led_names) / sizeof(led_names[0]);
    new_zone.leds_max   = sizeof(led_names) / sizeof(led_names[0]);
    new_zone.leds_count = sizeof(led_names) / sizeof(led_names[0]);
    new_zone.matrix_map.Set(6, 18, (unsigned int *)&matrix_map_firefly);
    zones.push_back(new_zone);

    for(unsigned int led_idx = 0; led_idx < new_zone.leds_count; led_idx++)
    {
        led new_led;
        new_led.name = led_names[led_idx];
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBcontroller_CosmicByteFireflyKeyboard::DeviceUpdateLEDs()
{
    LOG_DEBUG("[Firefly RGBController] DeviceUpdateLEDs triggered! Active mode index: %d", active_mode);

    if(modes[active_mode].color_mode == MODE_COLORS_PER_LED)
    {
        LOG_DEBUG("[Firefly RGBController] Mode is Direct! Invoking SendKeyboardColors(%d keys)...", (int)colors.size());
        controller->SendKeyboardColors(colors);
    }
    else
    {
        LOG_DEBUG("[Firefly RGBController] DeviceUpdateLEDs skipped: Active mode color_mode is not MODE_COLORS_PER_LED");
    }
}

void RGBcontroller_CosmicByteFireflyKeyboard::DeviceUpdateZoneLEDs(int zone)
{
    LOG_DEBUG("[Firefly RGBController] DeviceUpdateZoneLEDs triggered for zone %d", zone);
    DeviceUpdateLEDs();
}

void RGBcontroller_CosmicByteFireflyKeyboard::DeviceUpdateSingleLED(int led)
{
    LOG_DEBUG("[Firefly RGBController] DeviceUpdateSingleLED triggered for LED %d", led);
    DeviceUpdateLEDs();
}

void RGBcontroller_CosmicByteFireflyKeyboard::DeviceUpdateMode()
{
    if(active_mode >= modes.size())
    {
        return;
    }

    mode& current_mode = modes[active_mode];

    LOG_DEBUG("[Firefly RGBController] DeviceUpdateMode called for mode: %s", current_mode.name.c_str());

    if(current_mode.color_mode == MODE_COLORS_PER_LED)
    {
        // Direct Per-Key Mode
        DeviceUpdateLEDs();
    }
    else if(current_mode.value == 0x00)
    {
        // Global Static Mode
        RGBColor static_color    = current_mode.colors[0];
        unsigned char red        = RGBGetRValue(static_color);
        unsigned char green      = RGBGetGValue(static_color);
        unsigned char blue       = RGBGetBValue(static_color);
        unsigned char brightness = (unsigned char)current_mode.brightness;

        LOG_DEBUG("[Firefly RGBController] Updating Static Mode -> R: %d G: %d B: %d (Brightness: %d)", (int)red, (int)green, (int)blue, (int)brightness);

        controller->SendInitialize();
        controller->SendStaticColorPayload(red, green, blue);
        controller->SendControlInstruction(brightness, 0x01, 0x00, 0x00, 0x00);
        controller->SendTerminate();
    }
    else
    {
        // Hardware Animations
        unsigned char anim_type  = (unsigned char)current_mode.value;
        unsigned char brightness = (unsigned char)current_mode.brightness;
        unsigned char speed      = (unsigned char)current_mode.speed;
        unsigned char direction  = (current_mode.direction == MODE_DIRECTION_LEFT) ? 0x01 : 0x00;
        unsigned char slider_no  = 0x05;

        LOG_DEBUG("[Firefly RGBController] Updating Animation Mode: %s (ID: 0x%02X, Brightness: %d, Speed: %d)", current_mode.name.c_str(), (int)anim_type, (int)brightness, (int)speed);

        controller->SendInitialize();
        controller->SendControlInstruction(brightness, speed, direction, anim_type, slider_no);
        controller->SendTerminate();
    }
}