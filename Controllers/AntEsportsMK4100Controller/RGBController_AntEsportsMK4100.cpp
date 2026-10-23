/*---------------------------------------------------------*\
| RGBController_AntEsportsMK4100.cpp                        |
|                                                           |
| OpenRGB RGB controller for Ant Esports MK4100             |
|                                                           |
| This file is part of the OpenRGB project                  |
| SPDX-License-Identifier: GPL-2.0-or-later                 |
\*---------------------------------------------------------*/

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <vector>

#include "LogManager.h"
#include "RGBControllerKeyNames.h"
#include "RGBController_AntEsportsMK4100.h"


struct AntEsportsMK4100KeyPosition
{
    const char* name;
    unsigned slot;
    float x;
    float y;
    float width;
    float height;
};

static const AntEsportsMK4100KeyPosition key_positions[] = {
    {"Esc", 0u, 0.0000f, 0.0000f, 1.0000f, 1.0000f},
    {"F1", 1u, 2.0000f, 0.0000f, 1.0000f, 1.0000f},
    {"F2", 2u, 3.0000f, 0.0000f, 1.0000f, 1.0000f},
    {"F3", 3u, 4.0000f, 0.0000f, 1.0000f, 1.0000f},
    {"F4", 4u, 5.0000f, 0.0000f, 1.0000f, 1.0000f},
    {"F5", 5u, 6.5000f, 0.0000f, 1.0000f, 1.0000f},
    {"F6", 6u, 7.5000f, 0.0000f, 1.0000f, 1.0000f},
    {"F7", 7u, 8.5000f, 0.0000f, 1.0000f, 1.0000f},
    {"F8", 8u, 9.5000f, 0.0000f, 1.0000f, 1.0000f},
    {"F9", 9u, 11.0000f, 0.0000f, 1.0000f, 1.0000f},
    {"F10", 10u, 12.0000f, 0.0000f, 1.0000f, 1.0000f},
    {"F11", 11u, 13.0000f, 0.0000f, 1.0000f, 1.0000f},
    {"F12", 12u, 14.0000f, 0.0000f, 1.0000f, 1.0000f},
    {"Print", 13u, 15.5000f, 0.0000f, 1.0000f, 1.0000f},
    {"ScrLock", 14u, 16.5000f, 0.0000f, 1.0000f, 1.0000f},
    {"Pause", 15u, 17.5000f, 0.0000f, 1.0000f, 1.0000f},
    {"~", 21u, 0.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"1", 22u, 1.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"2", 23u, 2.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"3", 24u, 3.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"4", 25u, 4.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"5", 26u, 5.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"6", 27u, 6.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"7", 28u, 7.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"8", 29u, 8.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"9", 30u, 9.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"0", 31u, 10.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"-", 32u, 11.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"+", 33u, 12.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"Back", 34u, 13.0000f, 1.2500f, 2.0000f, 1.0000f},
    {"Insert", 35u, 15.5000f, 1.2500f, 1.0000f, 1.0000f},
    {"Home", 36u, 16.5000f, 1.2500f, 1.0000f, 1.0000f},
    {"PgUp", 37u, 17.5000f, 1.2500f, 1.0000f, 1.0000f},
    {"NumLock", 38u, 19.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"Num /", 39u, 20.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"Num *", 40u, 21.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"Num -", 41u, 22.0000f, 1.2500f, 1.0000f, 1.0000f},
    {"Tab", 42u, 0.0000f, 2.2500f, 1.5000f, 1.0000f},
    {"Q", 43u, 1.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"W", 44u, 2.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"E", 45u, 3.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"R", 46u, 4.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"T", 47u, 5.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"Y", 48u, 6.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"U", 49u, 7.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"I", 50u, 8.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"O", 51u, 9.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"P", 52u, 10.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"[", 53u, 11.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"]", 54u, 12.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"\\", 55u, 13.5000f, 2.2500f, 1.5000f, 1.0000f},
    {"Delet", 56u, 15.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"End", 57u, 16.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"PgDown", 58u, 17.5000f, 2.2500f, 1.0000f, 1.0000f},
    {"Num 7", 59u, 19.0000f, 2.2500f, 1.0000f, 1.0000f},
    {"Num 8", 60u, 20.0000f, 2.2500f, 1.0000f, 1.0000f},
    {"Num 9", 61u, 21.0000f, 2.2500f, 1.0000f, 1.0000f},
    {"Caps", 63u, 0.0000f, 3.2500f, 1.7500f, 1.0000f},
    {"A", 64u, 1.7500f, 3.2500f, 1.0000f, 1.0000f},
    {"S", 65u, 2.7500f, 3.2500f, 1.0000f, 1.0000f},
    {"D", 66u, 3.7500f, 3.2500f, 1.0000f, 1.0000f},
    {"F", 67u, 4.7500f, 3.2500f, 1.0000f, 1.0000f},
    {"G", 68u, 5.7500f, 3.2500f, 1.0000f, 1.0000f},
    {"H", 69u, 6.7500f, 3.2500f, 1.0000f, 1.0000f},
    {"J", 70u, 7.7500f, 3.2500f, 1.0000f, 1.0000f},
    {"K", 71u, 8.7500f, 3.2500f, 1.0000f, 1.0000f},
    {"L", 72u, 9.7500f, 3.2500f, 1.0000f, 1.0000f},
    {";", 73u, 10.7500f, 3.2500f, 1.0000f, 1.0000f},
    {"'", 74u, 11.7500f, 3.2500f, 1.0000f, 1.0000f},
    {"Enter", 76u, 12.7500f, 3.2500f, 2.2500f, 1.0000f},
    {"Num 4", 80u, 19.0000f, 3.2500f, 1.0000f, 1.0000f},
    {"Num 5", 81u, 20.0000f, 3.2500f, 1.0000f, 1.0000f},
    {"Num 6", 82u, 21.0000f, 3.2500f, 1.0000f, 1.0000f},
    {"Num +", 62u, 22.0000f, 2.2500f, 1.0000f, 2.0000f},
    {"Left Shift", 84u, 0.0000f, 4.2500f, 2.2500f, 1.0000f},
    {"Z", 86u, 2.2500f, 4.2500f, 1.0000f, 1.0000f},
    {"X", 87u, 3.2500f, 4.2500f, 1.0000f, 1.0000f},
    {"C", 88u, 4.2500f, 4.2500f, 1.0000f, 1.0000f},
    {"V", 89u, 5.2500f, 4.2500f, 1.0000f, 1.0000f},
    {"B", 90u, 6.2500f, 4.2500f, 1.0000f, 1.0000f},
    {"N", 91u, 7.2500f, 4.2500f, 1.0000f, 1.0000f},
    {"M", 92u, 8.2500f, 4.2500f, 1.0000f, 1.0000f},
    {"<", 93u, 9.2500f, 4.2500f, 1.0000f, 1.0000f},
    {">", 94u, 10.2500f, 4.2500f, 1.0000f, 1.0000f},
    {"?", 95u, 11.2500f, 4.2500f, 1.0000f, 1.0000f},
    {"Right Shift", 97u, 12.2500f, 4.2500f, 2.7500f, 1.0000f},
    {"Up", 99u, 16.5000f, 4.2500f, 1.0000f, 1.0000f},
    {"Num 1", 101u, 19.0000f, 4.2500f, 1.0000f, 1.0000f},
    {"Num 2", 102u, 20.0000f, 4.2500f, 1.0000f, 1.0000f},
    {"Num 3", 103u, 21.0000f, 4.2500f, 1.0000f, 1.0000f},
    {"Num Enter", 104u, 22.0000f, 4.2500f, 1.0000f, 2.0000f},
    {"Left Ctrl", 105u, 0.0000f, 5.2500f, 1.2500f, 1.0000f},
    {"Win", 106u, 1.2500f, 5.2500f, 1.2500f, 1.0000f},
    {"Left Alt", 107u, 2.5000f, 5.2500f, 1.2500f, 1.0000f},
    {"Space", 111u, 3.7500f, 5.2500f, 6.2500f, 1.0000f},
    {"Right Alt", 115u, 10.0000f, 5.2500f, 1.5000f, 1.0000f},
    {"Fn", 116u, 11.5000f, 5.2500f, 1.0000f, 1.0000f},
    {"App", 117u, 12.5000f, 5.2500f, 1.0000f, 1.0000f},
    {"Right Ctrl", 118u, 13.5000f, 5.2500f, 1.5000f, 1.0000f},
    {"Left", 119u, 15.5000f, 5.2500f, 1.0000f, 1.0000f},
    {"Down", 120u, 16.5000f, 5.2500f, 1.0000f, 1.0000f},
    {"Right", 121u, 17.5000f, 5.2500f, 1.0000f, 1.0000f},
    {"Num 0", 123u, 19.0000f, 5.2500f, 2.0000f, 1.0000f},
    {"Num .", 124u, 21.0000f, 5.2500f, 1.0000f, 1.0000f},
};

static const AntEsportsMK4100KeyPosition* GetKeyPositions(size_t* count)
{
    if(count) *count = sizeof(key_positions) / sizeof(key_positions[0]);
    return key_positions;
}


static constexpr unsigned int MATRIX_ROWS = 6;
static constexpr unsigned int MATRIX_COLUMNS = 23; /* one key-unit column; use OpenRGB's native wide-key rendering */
static constexpr unsigned int DIRECT_MODE_VALUE = MK4100_CUSTOM_MODE;

static const char* OpenRGBKeyName(const char* name)
{
    /* Use OpenRGB's standard keyboard identifiers so its keycap label lookup
       displays familiar legends instead of treating the keys as generic LEDs. */
    struct key_name_alias
    {
        const char* layout_name;
        const char* openrgb_name;
    };
    static const key_name_alias aliases[] =
    {
        {"Esc", KEY_EN_ESCAPE}, {"F1", KEY_EN_F1}, {"F2", KEY_EN_F2},
        {"F3", KEY_EN_F3}, {"F4", KEY_EN_F4}, {"F5", KEY_EN_F5},
        {"F6", KEY_EN_F6}, {"F7", KEY_EN_F7}, {"F8", KEY_EN_F8},
        {"F9", KEY_EN_F9}, {"F10", KEY_EN_F10}, {"F11", KEY_EN_F11},
        {"F12", KEY_EN_F12}, {"Print", KEY_EN_PRINT_SCREEN},
        {"ScrLock", KEY_EN_SCROLL_LOCK}, {"Pause", KEY_EN_PAUSE_BREAK},
        {"~", KEY_EN_BACK_TICK}, {"1", KEY_EN_1}, {"2", KEY_EN_2},
        {"3", KEY_EN_3}, {"4", KEY_EN_4}, {"5", KEY_EN_5},
        {"6", KEY_EN_6}, {"7", KEY_EN_7}, {"8", KEY_EN_8},
        {"9", KEY_EN_9}, {"0", KEY_EN_0}, {"-", KEY_EN_MINUS},
        /* The main keycap is = with + as its shifted legend. */
        {"+", KEY_EN_EQUALS}, {"Back", KEY_EN_BACKSPACE},
        {"Insert", KEY_EN_INSERT}, {"Home", KEY_EN_HOME},
        {"PgUp", KEY_EN_PAGE_UP}, {"NumLock", KEY_EN_NUMPAD_LOCK},
        {"Num /", KEY_EN_NUMPAD_DIVIDE}, {"Num *", KEY_EN_NUMPAD_TIMES},
        {"Num -", KEY_EN_NUMPAD_MINUS}, {"Tab", KEY_EN_TAB},
        {"Q", KEY_EN_Q}, {"W", KEY_EN_W}, {"E", KEY_EN_E},
        {"R", KEY_EN_R}, {"T", KEY_EN_T}, {"Y", KEY_EN_Y},
        {"U", KEY_EN_U}, {"I", KEY_EN_I}, {"O", KEY_EN_O},
        {"P", KEY_EN_P}, {"[", KEY_EN_LEFT_BRACKET},
        {"]", KEY_EN_RIGHT_BRACKET}, {"\\", KEY_EN_ANSI_BACK_SLASH},
        {"Delet", KEY_EN_DELETE}, {"End", KEY_EN_END},
        {"PgDown", KEY_EN_PAGE_DOWN}, {"Num 7", KEY_EN_NUMPAD_7},
        {"Num 8", KEY_EN_NUMPAD_8}, {"Num 9", KEY_EN_NUMPAD_9},
        {"Caps", KEY_EN_CAPS_LOCK}, {"A", KEY_EN_A}, {"S", KEY_EN_S},
        {"D", KEY_EN_D}, {"F", KEY_EN_F}, {"G", KEY_EN_G},
        {"H", KEY_EN_H}, {"J", KEY_EN_J}, {"K", KEY_EN_K},
        {"L", KEY_EN_L}, {";", KEY_EN_SEMICOLON}, {"'", KEY_EN_QUOTE},
        {"Enter", KEY_EN_ANSI_ENTER}, {"Num 4", KEY_EN_NUMPAD_4},
        {"Num 5", KEY_EN_NUMPAD_5}, {"Num 6", KEY_EN_NUMPAD_6},
        {"Num +", KEY_EN_NUMPAD_PLUS}, {"Left Shift", KEY_EN_LEFT_SHIFT},
        {"Z", KEY_EN_Z}, {"X", KEY_EN_X}, {"C", KEY_EN_C},
        {"V", KEY_EN_V}, {"B", KEY_EN_B}, {"N", KEY_EN_N},
        {"M", KEY_EN_M}, {"<", KEY_EN_COMMA}, {">", KEY_EN_PERIOD},
        {"?", KEY_EN_FORWARD_SLASH}, {"Right Shift", KEY_EN_RIGHT_SHIFT},
        {"Up", KEY_EN_UP_ARROW}, {"Num 1", KEY_EN_NUMPAD_1},
        {"Num 2", KEY_EN_NUMPAD_2}, {"Num 3", KEY_EN_NUMPAD_3},
        {"Num Enter", KEY_EN_NUMPAD_ENTER},
        {"Left Ctrl", KEY_EN_LEFT_CONTROL}, {"Win", KEY_EN_LEFT_WINDOWS},
        {"Left Alt", KEY_EN_LEFT_ALT}, {"Space", KEY_EN_SPACE},
        {"Right Alt", KEY_EN_RIGHT_ALT}, {"Fn", KEY_EN_RIGHT_FUNCTION},
        {"App", KEY_EN_MENU}, {"Right Ctrl", KEY_EN_RIGHT_CONTROL},
        {"Left", KEY_EN_LEFT_ARROW}, {"Down", KEY_EN_DOWN_ARROW},
        {"Right", KEY_EN_RIGHT_ARROW}, {"Num 0", KEY_EN_NUMPAD_0},
        {"Num .", KEY_EN_NUMPAD_PERIOD}
    };

    for(std::size_t i = 0; i < sizeof(aliases) / sizeof(aliases[0]); ++i)
    {
        if(std::strcmp(name, aliases[i].layout_name) == 0)
        {
            return aliases[i].openrgb_name;
        }
    }

    return name;
}

static void HSVBytesToRGB(uint8_t hue_byte, uint8_t saturation_byte, uint8_t value_byte,
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

static void RGBToHSVBytes(RGBColor color, uint8_t& hue_byte, uint8_t& saturation_byte, uint8_t& value_byte)
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

static void LoadModeValuesFromConfig(mode& target, const uint8_t config[MK4100_CONFIG_BYTES])
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
RGBController_AntEsportsMK4100::RGBController_AntEsportsMK4100(AntEsportsMK4100Controller* controller_ptr)
{
    controller = controller_ptr;
    last_mode_index = -1;
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
    const AntEsportsMK4100KeyPosition* keys = GetKeyPositions(&key_count);

    std::vector<unsigned int> matrix(MATRIX_ROWS * MATRIX_COLUMNS, std::numeric_limits<unsigned int>::max());
    for(std::size_t i = 0; i < key_count; ++i)
    {
        /* Store each LED in its key-unit cell. Standardized key names let OpenRGB 1.0
           expand wide keys in its native matrix-layout renderer. */
        const int col = static_cast<int>(std::lround(keys[i].x));
        const int row = static_cast<int>(std::lround(keys[i].y));
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
        new_led.name = OpenRGBKeyName(keys[i].name);
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
    const AntEsportsMK4100KeyPosition* keys = GetKeyPositions(&key_count);
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
    const AntEsportsMK4100KeyPosition* keys = GetKeyPositions(&key_count);
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
    uint8_t previous[MK4100_RGB_BYTES];
    if(!BuildFrame(frame)) return;
    controller->CopyCurrentRGB(previous);

    std::size_t key_count = 0;
    const AntEsportsMK4100KeyPosition* keys = GetKeyPositions(&key_count);
    if(key_count != colors.size()) return;

    std::vector<std::size_t> changed_leds;
    changed_leds.reserve(key_count);
    for(std::size_t i = 0; i < key_count; ++i)
    {
        const std::size_t offset = static_cast<std::size_t>(keys[i].slot) * 3u;
        if(std::memcmp(frame + offset, previous + offset, 3u) != 0)
        {
            changed_leds.push_back(i);
        }
    }

    /* Small manual edits use the vendor's acknowledged per-key 0x14 path.
       This avoids touching adjacent RGB slots at chunk boundaries. At six keys
       the expected latency is still about the same as one full bulk frame. */
    constexpr std::size_t SINGLE_KEY_UPDATE_LIMIT = 6u;
    if(changed_leds.empty()) return;
    if(changed_leds.size() <= SINGLE_KEY_UPDATE_LIMIT)
    {
        for(std::size_t changed_index = 0; changed_index < changed_leds.size(); ++changed_index)
        {
            const std::size_t i = changed_leds[changed_index];
            const std::size_t offset = static_cast<std::size_t>(keys[i].slot) * 3u;
            if(!controller->SetSingleKeyRGB(keys[i].slot, frame[offset], frame[offset + 1u], frame[offset + 2u]))
            {
                LOG_WARNING("[Ant Esports MK4100] Single-key update failed for %s: %s",
                            keys[i].name, controller->GetErrorString().c_str());
            }
        }
        return;
    }

    /* Larger edits and animations keep the bulk path, which uses 54-byte
       RGB-aligned chunks plus the protocol's overlap/boundary workarounds. */
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
    const AntEsportsMK4100KeyPosition* keys = GetKeyPositions(&key_count);
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
