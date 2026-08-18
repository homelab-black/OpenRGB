/*---------------------------------------------------------*\
| RGBController_AulaHero68.cpp                              |
|                                                           |
|   RGBController for AULA Hero 68 keyboard                 |
|                                                           |
|   Original author: dunn1o, 2026-08-09                     |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <chrono>
#include <thread>

#include "RGBController_AulaHero68.h"

#include "RGBControllerKeyNames.h"

#define NA 0xFFFFFFFF

namespace
{
uint64_t HashColors(const std::vector<RGBColor>& colors)
{
    uint64_t hash = 1469598103934665603ULL;

    for(size_t color_index = 0; color_index < colors.size(); color_index++)
    {
        hash ^= static_cast<uint64_t>(colors[color_index]);
        hash *= 1099511628211ULL;
    }

    return hash;
}
}

static unsigned int matrix_map[5][16] =
{ {  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 },
  { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, NA, 30 },
  { 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, NA, NA, 44 },
  { 45, NA, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, NA, 57, NA },
  { 58, 59, 60, NA, NA, 61, NA, NA, NA, 62, 63, 64, NA, 65, 66, 67 } };

typedef struct
{
    const char*     name;
    unsigned short  key_id;
} aula_hero68_led_type;

static const aula_hero68_led_type led_names[] =
{
    { KEY_EN_ESCAPE,            0x0001 },
    { KEY_EN_1,                 0x000F },
    { KEY_EN_2,                 0x0010 },
    { KEY_EN_3,                 0x0011 },
    { KEY_EN_4,                 0x0012 },
    { KEY_EN_5,                 0x0013 },
    { KEY_EN_6,                 0x0014 },
    { KEY_EN_7,                 0x0015 },
    { KEY_EN_8,                 0x0016 },
    { KEY_EN_9,                 0x0017 },
    { KEY_EN_0,                 0x0018 },
    { KEY_EN_MINUS,             0x0019 },
    { KEY_EN_EQUALS,            0x001A },
    { KEY_EN_BACKSPACE,         0x001B },
    { KEY_EN_INSERT,            0x0062 },
    { KEY_EN_DELETE,            0x0063 },
    { KEY_EN_TAB,               0x001C },
    { KEY_EN_Q,                 0x001D },
    { KEY_EN_W,                 0x001E },
    { KEY_EN_E,                 0x001F },
    { KEY_EN_R,                 0x0020 },
    { KEY_EN_T,                 0x0021 },
    { KEY_EN_Y,                 0x0022 },
    { KEY_EN_U,                 0x0023 },
    { KEY_EN_I,                 0x0024 },
    { KEY_EN_O,                 0x0025 },
    { KEY_EN_P,                 0x0026 },
    { KEY_EN_LEFT_BRACKET,      0x0027 },
    { KEY_EN_RIGHT_BRACKET,     0x0028 },
    { KEY_EN_ANSI_BACK_SLASH,   0x0029 },
    { KEY_EN_PAGE_UP,           0x0066 },
    { KEY_EN_CAPS_LOCK,         0x002A },
    { KEY_EN_A,                 0x002B },
    { KEY_EN_S,                 0x002C },
    { KEY_EN_D,                 0x002D },
    { KEY_EN_F,                 0x002E },
    { KEY_EN_G,                 0x002F },
    { KEY_EN_H,                 0x0030 },
    { KEY_EN_J,                 0x0031 },
    { KEY_EN_K,                 0x0032 },
    { KEY_EN_L,                 0x0033 },
    { KEY_EN_SEMICOLON,         0x0034 },
    { KEY_EN_QUOTE,             0x0035 },
    { KEY_EN_ANSI_ENTER,        0x0036 },
    { KEY_EN_PAGE_DOWN,         0x0067 },
    { KEY_EN_LEFT_SHIFT,        0x0037 },
    { KEY_EN_Z,                 0x0038 },
    { KEY_EN_X,                 0x0039 },
    { KEY_EN_C,                 0x003A },
    { KEY_EN_V,                 0x003B },
    { KEY_EN_B,                 0x003C },
    { KEY_EN_N,                 0x003D },
    { KEY_EN_M,                 0x003E },
    { KEY_EN_COMMA,             0x003F },
    { KEY_EN_PERIOD,            0x0040 },
    { KEY_EN_FORWARD_SLASH,     0x0041 },
    { KEY_EN_RIGHT_SHIFT,       0x0042 },
    { KEY_EN_UP_ARROW,          0x004A },
    { KEY_EN_LEFT_CONTROL,      0x0043 },
    { KEY_EN_LEFT_WINDOWS,      0x0044 },
    { KEY_EN_LEFT_ALT,          0x0045 },
    { KEY_EN_SPACE,             0x0046 },
    { KEY_EN_RIGHT_ALT,         0x0047 },
    { KEY_EN_RIGHT_FUNCTION,    0x0048 },
    { KEY_EN_RIGHT_CONTROL,     0x0049 },
    { KEY_EN_LEFT_ARROW,        0x004C },
    { KEY_EN_DOWN_ARROW,        0x004B },
    { KEY_EN_RIGHT_ARROW,       0x004D },
};

/**------------------------------------------------------------------*\
    @name AULA Hero 68
    @category Keyboard
    @type USB
    @save :x:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectAulaHero68Controllers
    @comment
\*-------------------------------------------------------------------*/

RGBController_AulaHero68::RGBController_AulaHero68(AulaHero68Controller* controller_ptr)
{
    controller  = controller_ptr;
    direct_mode_initialized = false;
    last_direct_frame_time = std::chrono::steady_clock::time_point::min();
    last_bulk_signature_time = std::chrono::steady_clock::time_point::min();
    last_bulk_signature = 0;

    name        = "AULA Hero 68";
    vendor      = "AULA";
    type        = DEVICE_TYPE_KEYBOARD;
    description = controller->GetDeviceName();
    serial      = controller->GetSerialString();
    location    = controller->GetDeviceLocation();

    mode Direct;
    Direct.name       = "Direct";
    Direct.value      = 0x13;
    Direct.flags      = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    mode Static;
    Static.name           = "Static";
    Static.value          = 0x01;
    Static.flags          = MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Static.color_mode     = MODE_COLORS_MODE_SPECIFIC;
    Static.colors_min     = 1;
    Static.colors_max     = 1;
    Static.colors.resize(1);
    Static.brightness_min = 0x01;
    Static.brightness_max = 0x14;
    Static.brightness     = 0x14;
    modes.push_back(Static);

    mode Reactive;
    Reactive.name       = "Reactive";
    Reactive.value      = 0x02;
    Reactive.flags      = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
    Reactive.color_mode = MODE_COLORS_MODE_SPECIFIC;
    Reactive.colors_min = 1;
    Reactive.colors_max = 1;
    Reactive.colors.resize(1);
    Reactive.speed_min  = 0x00;
    Reactive.speed_max  = 0x04;
    Reactive.speed      = 0x02;
    modes.push_back(Reactive);

    mode Off;
    Off.name       = "Off";
    Off.value      = 0x00;
    Off.flags      = 0;
    Off.color_mode = MODE_COLORS_NONE;
    modes.push_back(Off);

    SetupZones();
    current_colors = colors;
}

RGBController_AulaHero68::~RGBController_AulaHero68()
{
    delete controller;
}

void RGBController_AulaHero68::SetupZones()
{
    zone keyboard_zone;
    keyboard_zone.name                   = ZONE_EN_KEYBOARD;
    keyboard_zone.type                   = ZONE_TYPE_MATRIX;
    keyboard_zone.leds_min               = 68;
    keyboard_zone.leds_max               = 68;
    keyboard_zone.leds_count             = 68;
    keyboard_zone.matrix_map.Set(5, 16, (unsigned int*)&matrix_map);

    zones.push_back(keyboard_zone);

    for(size_t led_index = 0; led_index < sizeof(led_names) / sizeof(led_names[0]); led_index++)
    {
        const aula_hero68_led_type& led_desc = led_names[led_index];
        led new_led;
        new_led.name  = led_desc.name;
        new_led.value = led_desc.key_id;
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_AulaHero68::ResizeZone(int, int)
{
}

void RGBController_AulaHero68::DeviceUpdateLEDs()
{
    if(modes[active_mode].value != 0x13)
    {
        DeviceUpdateMode();
        return;
    }

    std::vector<std::size_t> changed_leds;

    for(std::size_t led_idx = 0; led_idx < leds.size(); led_idx++)
    {
        if(direct_mode_initialized && colors[led_idx] == current_colors[led_idx])
        {
            continue;
        }

        changed_leds.push_back(led_idx);
    }

    bool bulk_update = changed_leds.size() > 4;

    if(bulk_update)
    {
        std::chrono::steady_clock::time_point now =
            std::chrono::steady_clock::now();
        uint64_t bulk_signature = HashColors(colors);

        if(last_bulk_signature_time != std::chrono::steady_clock::time_point::min() &&
           bulk_signature == last_bulk_signature &&
           (now - last_bulk_signature_time) < std::chrono::milliseconds(50))
        {
            return;
        }

        if(last_direct_frame_time != std::chrono::steady_clock::time_point::min() &&
           (now - last_direct_frame_time) < std::chrono::milliseconds(33))
        {
            return;
        }

        last_direct_frame_time = now;
        last_bulk_signature_time = now;
        last_bulk_signature = bulk_signature;
    }

    if(!direct_mode_initialized)
    {
        controller->EnterCustomMode(0x00, 0x00, 0x00);
        std::this_thread::sleep_for(std::chrono::milliseconds(bulk_update ? 5 : 40));
    }

    if(bulk_update)
    {
        std::vector<AulaHero68KeyUpdate> updates;
        updates.reserve(changed_leds.size());

        for(size_t changed_index = 0; changed_index < changed_leds.size(); changed_index++)
        {
            size_t led_idx = changed_leds[changed_index];
            updates.push_back({ (uint16_t)leds[led_idx].value,
                                RGBGetRValue(colors[led_idx]),
                                RGBGetGValue(colors[led_idx]),
                                RGBGetBValue(colors[led_idx]) });
        }

        controller->SetKeysBatch(updates);
    }
    else
    {
        for(int pass = 0; pass < 2; pass++)
        {
            unsigned int writes_sent = 0;

            for(size_t changed_index = 0; changed_index < changed_leds.size(); changed_index++)
            {
                size_t led_idx = changed_leds[changed_index];
                controller->SetSingleKey((uint16_t)leds[led_idx].value,
                                         RGBGetRValue(colors[led_idx]),
                                         RGBGetGValue(colors[led_idx]),
                                         RGBGetBValue(colors[led_idx]),
                                         true);

                writes_sent++;

                if((writes_sent % 4) == 0)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(35));
                }
            }

            if(!changed_leds.empty() && pass == 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(80));
            }
        }
    }

    for(size_t changed_index = 0; changed_index < changed_leds.size(); changed_index++)
    {
        size_t led_idx = changed_leds[changed_index];
        current_colors[led_idx] = colors[led_idx];
    }

    direct_mode_initialized = true;
}

void RGBController_AulaHero68::UpdateZoneLEDs(int)
{
    DeviceUpdateLEDs();
}

void RGBController_AulaHero68::UpdateSingleLED(int led)
{
    if(modes[active_mode].value != 0x13)
    {
        DeviceUpdateLEDs();
        return;
    }

    controller->EnterCustomMode(0x00, 0x00, 0x00);
    controller->SetSingleKey((uint16_t)leds[led].value,
                             RGBGetRValue(colors[led]),
                             RGBGetGValue(colors[led]),
                             RGBGetBValue(colors[led]));
    current_colors[led] = colors[led];
    direct_mode_initialized = true;
}

void RGBController_AulaHero68::DeviceUpdateMode()
{
    switch(modes[active_mode].value)
    {
        case 0x13:
            DeviceUpdateLEDs();
            break;
        case 0x01:
            direct_mode_initialized = false;
            controller->SetModeStatic(RGBGetRValue(modes[active_mode].colors[0]),
                                      RGBGetGValue(modes[active_mode].colors[0]),
                                      RGBGetBValue(modes[active_mode].colors[0]),
                                      (uint8_t)modes[active_mode].brightness);
            break;
        case 0x02:
            direct_mode_initialized = false;
            controller->SetModeReactive(RGBGetRValue(modes[active_mode].colors[0]),
                                        RGBGetGValue(modes[active_mode].colors[0]),
                                        RGBGetBValue(modes[active_mode].colors[0]),
                                        (uint8_t)modes[active_mode].speed);
            break;
        default:
            direct_mode_initialized = false;
            controller->SetModeOff();
            break;
    }
}
