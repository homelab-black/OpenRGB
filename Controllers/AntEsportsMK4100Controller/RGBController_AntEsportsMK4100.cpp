/*---------------------------------------------------------*\
| RGBController_AntEsportsMK4100.cpp                        |
|                                                           |
| OpenRGB RGB controller for Ant Esports MK4100             |
| SPDX-License-Identifier: GPL-2.0-or-later                 |
\*---------------------------------------------------------*/

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

#include "LogManager.h"
#include "RGBController_AntEsportsMK4100.h"
#include "mk4100_layout.h"

namespace
{
constexpr unsigned int MATRIX_ROWS = 6;
constexpr unsigned int MATRIX_COLUMNS = 46; /* half-key columns preserve the full-size layout */
constexpr unsigned int DIRECT_MODE_VALUE = MK4100_CUSTOM_MODE;

void HSVBytesToRGB(uint8_t hue_byte, uint8_t saturation_byte, uint8_t value_byte,
                   uint8_t& red, uint8_t& green, uint8_t& blue)
{
    const double hue = (static_cast<double>(hue_byte) / 255.0) * 360.0;
    const double saturation = static_cast<double>(saturation_byte) / 255.0;
    const double value = static_cast<double>(value_byte) / 255.0;
    const double chroma = value * saturation;
    const double h_prime = std::fmod(hue / 60.0, 6.0);
    const double x = chroma * (1.0 - std::fabs(std::fmod(h_prime, 2.0) - 1.0));
    double r = 0.0, g = 0.0, b = 0.0;

    if(h_prime < 1.0)      { r = chroma; g = x; }
    else if(h_prime < 2.0) { r = x; g = chroma; }
    else if(h_prime < 3.0) { g = chroma; b = x; }
    else if(h_prime < 4.0) { g = x; b = chroma; }
    else if(h_prime < 5.0) { r = x; b = chroma; }
    else                   { r = chroma; b = x; }

    const double m = value - chroma;
    red   = static_cast<uint8_t>(std::lround((r + m) * 255.0));
    green = static_cast<uint8_t>(std::lround((g + m) * 255.0));
    blue  = static_cast<uint8_t>(std::lround((b + m) * 255.0));
}

void RGBToHSVBytes(RGBColor color, uint8_t& hue_byte, uint8_t& saturation_byte, uint8_t& value_byte)
{
    const double r = static_cast<double>(RGBGetRValue(color)) / 255.0;
    const double g = static_cast<double>(RGBGetGValue(color)) / 255.0;
    const double b = static_cast<double>(RGBGetBValue(color)) / 255.0;
    const double maximum = std::max(r, std::max(g, b));
    const double minimum = std::min(r, std::min(g, b));
    const double delta = maximum - minimum;
    double hue = 0.0;

    if(delta > 0.0)
    {
        if(maximum == r)      hue = 60.0 * std::fmod((g - b) / delta, 6.0);
        else if(maximum == g) hue = 60.0 * (((b - r) / delta) + 2.0);
        else                  hue = 60.0 * (((r - g) / delta) + 4.0);
        if(hue < 0.0) hue += 360.0;
    }

    const double saturation = maximum == 0.0 ? 0.0 : delta / maximum;
    hue_byte = static_cast<uint8_t>(std::lround((hue / 360.0) * 255.0)) ;
    saturation_byte = static_cast<uint8_t>(std::lround(saturation * 255.0));
    value_byte = static_cast<uint8_t>(std::lround(maximum * 255.0));
}

void LoadModeValuesFromConfig(mode& target, const uint8_t config[MK4100_CONFIG_BYTES])
{
    if(target.flags & MODE_FLAG_HAS_BRIGHTNESS)
        target.brightness = std::min<unsigned int>(config[3], 4u);
    if(target.flags & MODE_FLAG_HAS_SPEED)
        target.speed = std::min<unsigned int>(config[4], 2u);
    if(target.flags & (MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD))
    {
        if(target.flags & MODE_FLAG_HAS_DIRECTION_UD)
            target.direction = config[5] ? MODE_DIRECTION_DOWN : MODE_DIRECTION_UP;
        else
            target.direction = config[5] ? MODE_DIRECTION_RIGHT : MODE_DIRECTION_LEFT;
    }
    if(!target.colors.empty())
    {
        uint8_t r = 0, g = 0, b = 0;
        HSVBytesToRGB(config[8], config[9], config[10], r, g, b);
        target.colors[0] = ToRGBColor(r, g, b);
        target.color_mode = config[6] ? MODE_COLORS_MODE_SPECIFIC : MODE_COLORS_RANDOM;
    }
}
}

RGBController_AntEsportsMK4100::RGBController_AntEsportsMK4100(AntEsportsMK4100Controller* controller_ptr)
    : controller(controller_ptr), last_mode_index(-1)
{
    name = "Ant Esports MK4100";
    vendor = "Ant Esports";
    type = DEVICE_TYPE_KEYBOARD;
    description = "Ant Esports MK4100 104-key RGB keyboard";
    location = controller->GetDeviceLocation();
    serial = controller->GetSerialString();
    version = "JYK200 104M";

    /* Direct first so the device's manually addressable RGB buffer is easy to find.
       Hardware effects use the numeric values from the vendor profile. */
    mode direct;
    direct.name = "Direct";
    direct.value = static_cast<int>(DIRECT_MODE_VALUE);
    direct.flags = MODE_FLAG_HAS_PER_LED_COLOR;
    direct.color_mode = MODE_COLORS_PER_LED;
    modes.push_back(direct);

    AddMode("Off", 0, false, false, false, 0);
    AddMode("Static", 1, true, false, true, 0);
    AddMode("Breathing", 2, true, true, true, 0);
    AddMode("Rotation", 3, true, true, false, 0);
    AddMode("Spectrum", 4, true, true, false, 0);
    AddMode("Left-Right Flow", 5, true, true, true, 1);
    AddMode("Bloom", 6, true, true, false, 1);
    AddMode("Up-Down Flow", 7, true, true, true, 2);
    AddMode("Shuttle", 8, true, true, true, 0);
    AddMode("Rain", 9, true, true, true, 0);
    AddMode("Meteor", 10, true, true, true, 0);
    AddMode("Ripple Fade-Out", 11, true, true, true, 0);
    AddMode("Ripple Persistent", 12, true, true, true, 0);
    AddMode("Keypress", 13, true, true, true, 0);
    AddMode("Sudoku", 14, true, true, true, 0);
    AddMode("Tide", 15, true, true, true, 0);

    SetupZones();
    LoadCurrentDeviceState();

    const unsigned int hardware_mode = controller->GetActiveHardwareMode();
    active_mode = 0;
    for(unsigned int i = 0; i < modes.size(); ++i)
    {
        if(static_cast<unsigned int>(modes[i].value) == hardware_mode)
        {
            active_mode = static_cast<int>(i);
            break;
        }
    }
    last_mode_index = active_mode;
}

RGBController_AntEsportsMK4100::~RGBController_AntEsportsMK4100()
{
    Shutdown();
    delete controller;
}

void RGBController_AntEsportsMK4100::AddMode(const char* mode_name, int value,
                                              bool has_brightness, bool has_speed,
                                              bool has_color, int direction_type)
{
    mode current;
    current.name = mode_name;
    current.value = value;
    current.flags = MODE_FLAG_REQUIRES_ENTIRE_DEVICE;

    if(has_brightness)
    {
        current.flags |= MODE_FLAG_HAS_BRIGHTNESS;
        current.brightness_min = 0;
        current.brightness_max = 4;
        current.brightness = 4;
    }

    if(has_speed)
    {
        current.flags |= MODE_FLAG_HAS_SPEED;
        current.speed_min = 0;
        current.speed_max = 2;
        current.speed = 1;
    }

    if(has_color)
    {
        /* The vendor's color byte is a monochrome/single-color toggle. OpenRGB's
           standard two-state color selector is mapped to that byte: specific color
           means monochrome on (1), its alternate means full-color (0). */
        current.flags |= MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR;
        current.colors_min = 1;
        current.colors_max = 1;
        current.colors.resize(1, ToRGBColor(255, 255, 255));
        current.color_mode = MODE_COLORS_MODE_SPECIFIC;
    }

    if(direction_type == 1)
    {
        current.flags |= MODE_FLAG_HAS_DIRECTION_LR;
        current.direction = MODE_DIRECTION_LEFT;
    }
    else if(direction_type == 2)
    {
        current.flags |= MODE_FLAG_HAS_DIRECTION_UD;
        current.direction = MODE_DIRECTION_UP;
    }

    modes.push_back(current);
}

void RGBController_AntEsportsMK4100::SetupZones()
{
    std::size_t key_count = 0;
    const mk4100_key_position* keys = mk4100_get_key_positions(&key_count);

    std::vector<unsigned int> matrix(MATRIX_ROWS * MATRIX_COLUMNS, std::numeric_limits<unsigned int>::max());
    for(std::size_t i = 0; i < key_count; ++i)
    {
        const int col = static_cast<int>(std::lround((2.0f * keys[i].x) + keys[i].width));
        const int row = static_cast<int>(std::lround(keys[i].y + ((keys[i].height - 1.0f) / 2.0f)));
        if(row >= 0 && row < static_cast<int>(MATRIX_ROWS) && col >= 0 && col < static_cast<int>(MATRIX_COLUMNS))
        {
            matrix[static_cast<std::size_t>(row) * MATRIX_COLUMNS + static_cast<std::size_t>(col)] = static_cast<unsigned int>(i);
        }
    }

    zone keyboard_zone;
    keyboard_zone.name = "Keyboard";
    keyboard_zone.type = ZONE_TYPE_MATRIX;
    keyboard_zone.leds_min = static_cast<unsigned int>(key_count);
    keyboard_zone.leds_max = static_cast<unsigned int>(key_count);
    keyboard_zone.leds_count = static_cast<unsigned int>(key_count);
    keyboard_zone.matrix_map.Set(MATRIX_ROWS, MATRIX_COLUMNS, matrix.data());
    zones.push_back(keyboard_zone);

    for(std::size_t i = 0; i < key_count; ++i)
    {
        led new_led;
        new_led.name = keys[i].name;
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_AntEsportsMK4100::LoadCurrentDeviceState()
{
    uint8_t config[MK4100_CONFIG_BYTES] = {0};
    uint8_t rgb[MK4100_RGB_BYTES] = {0};
    controller->CopyCurrentConfig(config);
    controller->CopyCurrentRGB(rgb);

    for(std::size_t i = 0; i < modes.size(); ++i)
    {
        LoadModeValuesFromConfig(modes[i], config);
    }

    std::size_t key_count = 0;
    const mk4100_key_position* keys = mk4100_get_key_positions(&key_count);
    const std::size_t limit = std::min(key_count, colors.size());
    for(std::size_t i = 0; i < limit; ++i)
    {
        const std::size_t offset = static_cast<std::size_t>(keys[i].slot) * 3u;
        colors[i] = ToRGBColor(rgb[offset], rgb[offset + 1], rgb[offset + 2]);
    }
}

bool RGBController_AntEsportsMK4100::BuildFrame(uint8_t rgb[MK4100_RGB_BYTES]) const
{
    if(!rgb || colors.size() != leds.size()) return false;
    std::memset(rgb, 0, MK4100_RGB_BYTES);
    std::size_t key_count = 0;
    const mk4100_key_position* keys = mk4100_get_key_positions(&key_count);
    if(key_count != colors.size()) return false;

    for(std::size_t i = 0; i < key_count; ++i)
    {
        const std::size_t offset = static_cast<std::size_t>(keys[i].slot) * 3u;
        rgb[offset]     = static_cast<uint8_t>(RGBGetRValue(colors[i]));
        rgb[offset + 1] = static_cast<uint8_t>(RGBGetGValue(colors[i]));
        rgb[offset + 2] = static_cast<uint8_t>(RGBGetBValue(colors[i]));
    }
    return true;
}

void RGBController_AntEsportsMK4100::DeviceUpdateLEDs()
{
    if(active_mode < 0 || active_mode >= static_cast<int>(modes.size()) ||
       static_cast<unsigned int>(modes[static_cast<std::size_t>(active_mode)].value) != DIRECT_MODE_VALUE)
    {
        return;
    }

    uint8_t frame[MK4100_RGB_BYTES];
    if(!BuildFrame(frame)) return;
    if(!controller->ApplyDirectFrame(frame, false))
    {
        LOG_WARNING("[Ant Esports MK4100] Direct RGB update failed: %s", controller->GetErrorString().c_str());
    }
}

void RGBController_AntEsportsMK4100::DeviceUpdateZoneLEDs(int)
{
    DeviceUpdateLEDs();
}

void RGBController_AntEsportsMK4100::DeviceUpdateSingleLED(int led_index)
{
    if(active_mode < 0 || active_mode >= static_cast<int>(modes.size()) ||
       static_cast<unsigned int>(modes[static_cast<std::size_t>(active_mode)].value) != DIRECT_MODE_VALUE)
    {
        return;
    }
    if(led_index < 0 || led_index >= static_cast<int>(leds.size())) return;

    std::size_t key_count = 0;
    const mk4100_key_position* keys = mk4100_get_key_positions(&key_count);
    if(static_cast<std::size_t>(led_index) >= key_count) return;
    const RGBColor color = colors[static_cast<std::size_t>(led_index)];

    if(!controller->SetSingleKeyRGB(keys[led_index].slot,
                                    static_cast<uint8_t>(RGBGetRValue(color)),
                                    static_cast<uint8_t>(RGBGetGValue(color)),
                                    static_cast<uint8_t>(RGBGetBValue(color))))
    {
        LOG_WARNING("[Ant Esports MK4100] Single-key RGB update failed: %s", controller->GetErrorString().c_str());
    }
}

bool RGBController_AntEsportsMK4100::WriteSelectedMode()
{
    if(active_mode < 0 || active_mode >= static_cast<int>(modes.size())) return false;
    mode& selected = modes[static_cast<std::size_t>(active_mode)];

    if(static_cast<unsigned int>(selected.value) == DIRECT_MODE_VALUE)
    {
        uint8_t frame[MK4100_RGB_BYTES];
        if(!BuildFrame(frame) || !controller->ApplyDirectFrame(frame, false)) return false;
        last_mode_index = active_mode;
        return true;
    }

    uint8_t config[MK4100_CONFIG_BYTES] = {0};
    const bool mode_changed = active_mode != last_mode_index;
    if(mode_changed)
    {
        /* The vendor's mode-selection path queries command 0x16 to load that
           effect's saved settings before writing command 0x0B. This prevents
           selecting a mode from blindly reusing the previous mode's parameters. */
        if(!controller->ReadEffectConfigForMode(static_cast<unsigned int>(selected.value), config)) return false;
        LoadModeValuesFromConfig(selected, config);
        config[2] = static_cast<uint8_t>(selected.value);
    }
    else
    {
        /* Every successful config write updates the protocol core's cache.
           Reuse that acknowledged state during same-mode UI adjustments instead
           of issuing another 0x0A read for every color-wheel/parameter event. */
        controller->CopyCurrentConfig(config);
        config[2] = static_cast<uint8_t>(selected.value);

        if(selected.value == 0)
        {
            config[6] = 0;
        }
        else
        {
            if(selected.flags & MODE_FLAG_HAS_BRIGHTNESS)
                config[3] = static_cast<uint8_t>(std::min(selected.brightness, 4u));
            if(selected.flags & MODE_FLAG_HAS_SPEED)
                config[4] = static_cast<uint8_t>(std::min(selected.speed, 2u));
            if(selected.flags & (MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD))
            {
                config[5] = static_cast<uint8_t>(
                    (selected.direction == MODE_DIRECTION_RIGHT || selected.direction == MODE_DIRECTION_DOWN) ? 1 : 0);
            }
            if(selected.flags & MODE_FLAG_HAS_RANDOM_COLOR)
            {
                config[6] = selected.color_mode == MODE_COLORS_MODE_SPECIFIC ? 1u : 0u;
            }
            if((selected.flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR) &&
               selected.color_mode == MODE_COLORS_MODE_SPECIFIC && !selected.colors.empty())
            {
                RGBToHSVBytes(selected.colors[0], config[8], config[9], config[10]);
            }
        }
    }

    if(selected.value == 0) config[6] = 0;
    if(!controller->WriteConfig(config)) return false;
    last_mode_index = active_mode;
    return true;
}

void RGBController_AntEsportsMK4100::DeviceUpdateMode()
{
    if(!WriteSelectedMode())
    {
        LOG_WARNING("[Ant Esports MK4100] Lighting mode update failed: %s", controller->GetErrorString().c_str());
    }
}
