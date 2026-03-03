/*---------------------------------------------------------*\
| AkkoController.cpp                                        |
|                                                           |
|   Driver for Akko keyboard                                |
|                                                           |
|   Bizzotto Nicola (nicola02nb)                05 Feb 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <math.h>
#include "AkkoController.h"
#include "KeyboardLayoutManager.h"
#include "LogManager.h"
#include "StringUtils.h"

#define AKKO_PACKET_LENGTH_WITH_REPORT 65

std::vector<unsigned int> akko_5075_values =
    {
    /*ESC   F1   F2   F3   F4   F5   F6   F7   F8   F9  F10  F11  F12  DEL      */
        0,   6,  12,  18,  24,  30,  36,  42,  48,  54,  60,  66,  72,
    /*BKTK   1    2    3    4    5    6    7    8    9    0    -    = BSPC HOME */
        1,   7,  13,  19,  25,  31,  37,  43,  49,  55,  61,  67,  73,  79,
    /*TAB    Q    W    E    R    T    Y    U    I    O    P    [    ]    \  PGUP */
        2,   8,  14,  20,  26,  32,  38,  44,  50,  56,  62,  68,  74,  80,
    /*CPLK   A    S    D    F    G    H    J    K    L    ;    "  POUN ENTR PGDW */
        3,   9,  15,  21,  27,  33,  39,  45,  51,  57,  63,  69,  81,  75,
    /*LSFT ISO\   Z    X    C    V    B    N    M    ,    .    /  RSFT ARWU  END */
        4,  10,  16,  22,  28,  34,  40,  46,  52,  58,  64,  70,  76,
    /*LCTL      LWIN LALT            SPC      RALT  RFN MENU RCTL ARWL ARWD  ARWR*/
        5,       17,  23,            41,       53,  59,  65,  71,
};

keyboard_keymap_overlay_values akko_5075_layout
{
    KEYBOARD_SIZE_SEVENTY_FIVE,
    {
        akko_5075_values,
        {
            {
                KEYBOARD_LAYOUT_ISO_QWERTY,
                {
                    /*-------------------------------------------------------------------------------------------------------------------------------------*\
                    | Edit Keys                                                                                                                             |
                    |   Zone,   Row,    Column,     Value,      Name,                       Alternate Name,             OpCode                              |
                    \*-------------------------------------------------------------------------------------------------------------------------------------*/
                    {   0,      3,      13,           0,        KEY_EN_UNUSED,              KEY_EN_UNUSED,              KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove ANSI Enter
                    {   0,      2,      13,          80,        KEY_EN_ISO_ENTER,           KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert ISO Enter
                }
            },
            /* Add more regional layout fixes here */
        }
    },
    {
        /*-------------------------------------------------------------------------------------------------------------------------------------*\
        | Edit Keys                                                                                                                             |
        |   Zone,   Row,    Column,     Value,      Key,                        Alternate Name,             OpCode,                             |
        \*-------------------------------------------------------------------------------------------------------------------------------------*/
        {   0,      4,      13,           0,        KEY_EN_UNUSED,              KEY_EN_UNUSED,              KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove Right Shift
        {   0,      4,      12,          76,        KEY_EN_RIGHT_SHIFT,         KEY_EN_UNUSED,              KEYBOARD_OPCODE_SWAP_ONLY,          },  // Move Right Shift
        {   0,      5,      12,           0,        KEY_EN_UNUSED,              KEY_EN_UNUSED,              KEYBOARD_OPCODE_REMOVE_SHIFT_LEFT,  },  // Remove Menu

        {   0,      0,      14,          78,        KEY_EN_DELETE,              KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert Delete
        {   0,      1,      15,          85,        KEY_EN_HOME,                KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert Home
        {   0,      2,      15,          86,        KEY_EN_PAGE_UP,             KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert Page Up
        {   0,      3,      15,          87,        KEY_EN_PAGE_DOWN,           KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert Page Down
        {   0,      4,      14,          82,        KEY_EN_UP_ARROW,            KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert Up Arrow
        {   0,      4,      15,          88,        KEY_EN_END,                 KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert End
        {   0,      5,      13,          77,        KEY_EN_LEFT_ARROW,          KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert Left Arrow
        {   0,      5,      14,          83,        KEY_EN_DOWN_ARROW,          KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert Down Arrow
        {   0,      5,      15,          89,        KEY_EN_RIGHT_ARROW,         KEY_EN_UNUSED,              KEYBOARD_OPCODE_INSERT_SHIFT_RIGHT, },  // Insert Right Arrow
    }
};

AkkoController::AkkoController(hid_device* dev_handle, char *_path, bool _is_wireless, bool _has_light_strips)
{
    dev         = dev_handle;
    location    = _path;

    is_wireless = _is_wireless;
    has_light_strips = _has_light_strips;

    /*---------------------------------------------------------*\
    | Get device name from HID manufacturer and product strings |
    \*---------------------------------------------------------*/
    wchar_t name_string[HID_MAX_STR];

    hid_get_manufacturer_string(dev, name_string, HID_MAX_STR);
    device_name = StringUtils::wstring_to_string(name_string);

    hid_get_product_string(dev, name_string, HID_MAX_STR);
    device_name.append(" ").append(StringUtils::wstring_to_string(name_string));
}

AkkoController::~AkkoController()
{
    hid_close(dev);
}

std::string AkkoController::GetDeviceName()
{
    return(device_name);
}

std::string AkkoController::GetSerial()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

std::string AkkoController::GetLocation()
{
    return("HID: " + location);
}

KEYBOARD_LAYOUT AkkoController::GetKeyboardLayout()
{
    return KEYBOARD_LAYOUT_ISO_QWERTY;
}

KEYBOARD_SIZE AkkoController::GetKeyboardSize()
{
    return KEYBOARD_SIZE_SEVENTY_FIVE;
}

keyboard_keymap_overlay_values AkkoController::GetKeyboardLayoutValues()
{
    return akko_5075_layout;
}

int calculateFiller(unsigned char *buffer, int start, int end)
{
    int sum_bits = 0;
    for(int i = start; i <= end; i++)
    {
        sum_bits += buffer[i];
    }

    int filler = 255 - sum_bits;
    return filler;
}

void AkkoController::SetZoneEffect(const AkkoZoneInfo& zone_info)
{
    if(!IsZoneCompatibleWith(zone_info.zone, zone_info.mode))
    {
        return; // Only keyboard supports all modes, light strips only support OFF to RIPPLE
    }

    AKKO_BYTE_COMMAND command = getByteFromZone(zone_info.zone);
    AKKO_MODE mode = zone_info.mode;
    AKKO_SPEED speed = zone_info.speed;
    AKKO_BRIGHTNESS brightness = zone_info.brightness;
    AKKO_FLAGS flags = zone_info.flags;
    RGBColor color = zone_info.color;

    if(mode == AKKO_MODE::USER_PICTURE || mode == AKKO_MODE::MUSIC_FOLLOW
        || mode == AKKO_MODE::MUSIC_FOLLOW_2 || mode == AKKO_MODE::LIGHT_SHADOW)
    {
        speed = AKKO_SPEED::S_MAX;
        brightness = AKKO_BRIGHTNESS::B_MAX;
        if(mode == AKKO_MODE::USER_PICTURE)
        {
            flags = AKKO_FLAGS::DEFAULT;
        }
    }
    else
    {
        if (zone_info.zone == AKKO_ZONE::Z_KEYBOARD)
        {
            speed = (AKKO_SPEED)(AKKO_SPEED::S_MAX - speed); // Invert speed (0 is max speed, 4 is min speed)
        }
    }

    unsigned char buffer[AKKO_PACKET_LENGTH_WITH_REPORT] = { 0x00 };
    buffer[AKKO_BYTE_INDEX::COMMAND]      = AKKO_BYTE_COMMAND::SET | command;
    buffer[AKKO_BYTE_INDEX::MODE]         = mode;
    buffer[AKKO_BYTE_INDEX::SPEED]        = speed;
    buffer[AKKO_BYTE_INDEX::BRIGHTNESS]   = brightness;
    buffer[AKKO_BYTE_INDEX::FLAGS]        = flags;
    buffer[AKKO_BYTE_INDEX::RED]          = RGBGetRValue(color);
    buffer[AKKO_BYTE_INDEX::GREEN]        = RGBGetGValue(color);
    buffer[AKKO_BYTE_INDEX::BLUE]         = RGBGetBValue(color);
    buffer[AKKO_BYTE_INDEX::EFFECT_FILLER]   = calculateFiller(buffer, AKKO_BYTE_INDEX::COMMAND, AKKO_BYTE_INDEX::BLUE);

    int send_buffer_result = hid_send_feature_report(dev, buffer, AKKO_PACKET_LENGTH_WITH_REPORT);
    if(send_buffer_result<0)
    {
        LOG_ERROR("[%s]: Send Buffer Error. HIDAPI Error: %ls", device_name.c_str(), hid_error(dev));
    }
}

AkkoZoneInfo AkkoController::GetZoneInfo(AKKO_ZONE zone)
{
    AkkoZoneInfo effect;

    AKKO_BYTE_COMMAND command = getByteFromZone(zone);
    unsigned char buffer[AKKO_PACKET_LENGTH_WITH_REPORT] = { 0x00 };
    buffer[AKKO_BYTE_INDEX::COMMAND]    = AKKO_BYTE_COMMAND::GET | command;
    buffer[AKKO_BYTE_INDEX::MODE]       = 0x00;
    buffer[AKKO_BYTE_INDEX::SPEED]      = 0x00;
    buffer[AKKO_BYTE_INDEX::BRIGHTNESS] = 0x00;
    buffer[AKKO_BYTE_INDEX::FLAGS]      = 0x00;
    buffer[AKKO_BYTE_INDEX::RED]        = 0x00;
    buffer[AKKO_BYTE_INDEX::GREEN]      = 0x00;
    buffer[AKKO_BYTE_INDEX::CUSTOM_FILLER]       = calculateFiller(buffer, AKKO_BYTE_INDEX::COMMAND, AKKO_BYTE_INDEX::GREEN);

    int send_buffer_result = hid_send_feature_report(dev, buffer, AKKO_PACKET_LENGTH_WITH_REPORT);
    if(send_buffer_result<0)
    {
        LOG_ERROR("[%s]: Get Buffer Error. HIDAPI Error: %ls", device_name.c_str(), hid_error(dev));
        return effect;
    }

    memset(buffer, 0, sizeof(buffer));
    int get_buffer_result = hid_get_feature_report(dev, buffer, AKKO_PACKET_LENGTH_WITH_REPORT);
    if(get_buffer_result<0)
    {
        LOG_ERROR("[%s]: Get Buffer Error. HIDAPI Error: %ls", device_name.c_str(), hid_error(dev));
        return effect;
    }

    effect.name = getZoneName(zone);
    effect.zone = zone;
    effect.mode = (AKKO_MODE)buffer[AKKO_BYTE_INDEX::MODE];
    effect.speed = (AKKO_SPEED)(AKKO_SPEED::S_MAX - buffer[AKKO_BYTE_INDEX::SPEED]);
    effect.brightness = (AKKO_BRIGHTNESS)buffer[AKKO_BYTE_INDEX::BRIGHTNESS];
    effect.flags = (AKKO_FLAGS)buffer[AKKO_BYTE_INDEX::FLAGS];
    effect.color = ToRGBColor(buffer[AKKO_BYTE_INDEX::RED], buffer[AKKO_BYTE_INDEX::GREEN], buffer[AKKO_BYTE_INDEX::BLUE]);

    if(zone == AKKO_ZONE::Z_KEYBOARD)
    {
        effect.colors = getLedsData(zone);
    }

    return effect;
}

void AkkoController::SetCustomLeds(const AkkoZoneInfo& zone_info)
{
    if(zone_info.zone != AKKO_ZONE::Z_KEYBOARD)
    {
        return; // Only keyboard supports custom per-LED colors
    }

    int pages = AKKO_ZONE_SIZE::Z_KEYBOARD_SIZE / ((AKKO_PACKET_LENGTH - AKKO_BYTE_INDEX::CUSTOM_FILLER) / 3);
    unsigned char buffer[AKKO_PACKET_LENGTH_WITH_REPORT] = { 0x00 };

    int led_index = 0;
    int color_byte_k = 0;

    for(int i = 0; i < pages; ++i)
    {
        memset(buffer, 0, sizeof(buffer));
        buffer[AKKO_BYTE_INDEX::COMMAND]       = AKKO_BYTE_COMMAND::SET | AKKO_BYTE_COMMAND::CUSTOM_KEYBOARD;
        buffer[AKKO_BYTE_INDEX::MODE]          = 0x00;
        buffer[AKKO_BYTE_INDEX::SPEED]         = 0x80;
        buffer[AKKO_BYTE_INDEX::BRIGHTNESS]    = 0x01;
        buffer[AKKO_BYTE_INDEX::FLAGS]         = i;
        buffer[AKKO_BYTE_INDEX::RED]           = 0x00;
        buffer[AKKO_BYTE_INDEX::GREEN]         = 0x00;
        buffer[AKKO_BYTE_INDEX::CUSTOM_FILLER] = calculateFiller(buffer, AKKO_BYTE_INDEX::COMMAND, AKKO_BYTE_INDEX::GREEN);

        int buffer_idx = AKKO_BYTE_INDEX::CUSTOM_FILLER + 1;

        while (buffer_idx < AKKO_PACKET_LENGTH_WITH_REPORT && led_index < zone_info.colors.size())
        {
            RGBColor current_color = zone_info.colors[led_index];
            buffer[buffer_idx] = (current_color >> (color_byte_k * 8)) & 0xFF;
            buffer_idx++;
            color_byte_k++;
            if(color_byte_k >= 3)
            {
                color_byte_k = 0;
                led_index++;
            }
        }

        int send_buffer_result = hid_send_feature_report(dev, buffer, AKKO_PACKET_LENGTH_WITH_REPORT);
        if(send_buffer_result < 0)
        {
            LOG_ERROR("[%s]: Send Buffer Error. HIDAPI Error: %ls", device_name.c_str(), hid_error(dev));
        }
    }
}

void AkkoController::SetDirectMusic(const AkkoZoneInfo& zone_info)
{
    if(zone_info.zone != AKKO_ZONE::Z_KEYBOARD || (zone_info.mode != AKKO_MODE::MUSIC_FOLLOW && zone_info.mode != AKKO_MODE::MUSIC_FOLLOW_2))
    {
        return; // Only keyboard supports direct music mode and only if the mode is MUSIC_FOLLOW or MUSIC_FOLLOW_2
    }
    unsigned char buffer[AKKO_PACKET_LENGTH_WITH_REPORT] = { 0x00 };

    buffer[AKKO_BYTE_INDEX::COMMAND]      = AKKO_BYTE_COMMAND::SET | AKKO_BYTE_COMMAND::DIRECT_MUSIC;
    buffer[AKKO_BYTE_INDEX::MODE]         = 0x0;
    buffer[AKKO_BYTE_INDEX::SPEED]        = 0x0;
    buffer[AKKO_BYTE_INDEX::BRIGHTNESS]   = 0x0;
    buffer[AKKO_BYTE_INDEX::FLAGS]        = 0x0;
    buffer[AKKO_BYTE_INDEX::RED]          = 0x0;
    buffer[AKKO_BYTE_INDEX::GREEN]        = 0x0;
    buffer[AKKO_BYTE_INDEX::BLUE]         = calculateFiller(buffer, AKKO_BYTE_INDEX::COMMAND, AKKO_BYTE_INDEX::GREEN);

    // Map colors to LED column heights (0-6) for each frequency band
    // Typically 32 values representing frequency spectrum from 473hz to 1540hz
    // Each value (0-6) represents how many LED rows from bottom to top should be lit
    int buffer_idx = AKKO_BYTE_INDEX::EFFECT_FILLER;
    int max_values = std::min(32, (int)zone_info.colors.size());

    for(int i = 0; i < max_values; i++)
    {
        RGBColor color = zone_info.colors[i];
        // Extract intensity from color and map to LED column height (0-6)
        // 0 = no LEDs lit, 6 = all LEDs in column lit from bottom to top
        int intensity = std::max({RGBGetRValue(color), RGBGetGValue(color), RGBGetBValue(color)});
        buffer[buffer_idx++] = (intensity * 7) / 256;
    }

    // Fill remaining bytes with 0
    while(buffer_idx < AKKO_PACKET_LENGTH_WITH_REPORT)
    {
        buffer[buffer_idx++] = 0;
    }

    int send_buffer_result = hid_send_feature_report(dev, buffer, AKKO_PACKET_LENGTH_WITH_REPORT);
    if(send_buffer_result<0)
    {
        LOG_ERROR("[%s]: Send Buffer Error. HIDAPI Error: %ls", device_name.c_str(), hid_error(dev));
    }
}

void AkkoController::SetDirectColor(const AkkoZoneInfo& zone_info)
{
    if(zone_info.zone != AKKO_ZONE::Z_KEYBOARD || zone_info.mode != AKKO_MODE::LIGHT_SHADOW)
    {
        return; // Only keyboard supports direct color mode, and only in LIGHT_SHADOW mode
    }
    unsigned char buffer[AKKO_PACKET_LENGTH_WITH_REPORT] = { 0x00 };

    unsigned int average_red = 0;
    unsigned int average_green = 0;
    unsigned int average_blue = 0;
    for (size_t i = 0; i < zone_info.colors.size(); i++)
    {
        average_red += RGBGetRValue(zone_info.colors[i]);
        average_green += RGBGetGValue(zone_info.colors[i]);
        average_blue += RGBGetBValue(zone_info.colors[i]);
    }
    if(!zone_info.colors.empty())
    {
        average_red /= zone_info.colors.size();
        average_green /= zone_info.colors.size();
        average_blue /= zone_info.colors.size();
    }

    buffer[AKKO_BYTE_INDEX::COMMAND]      = AKKO_BYTE_COMMAND::SET | AKKO_BYTE_COMMAND::DIRECT_COLOR;
    buffer[AKKO_BYTE_INDEX::MODE]         = average_red;
    buffer[AKKO_BYTE_INDEX::SPEED]        = average_green;
    buffer[AKKO_BYTE_INDEX::BRIGHTNESS]   = average_blue;
    buffer[AKKO_BYTE_INDEX::FLAGS]        = 0x0;
    buffer[AKKO_BYTE_INDEX::RED]          = 0x0;
    buffer[AKKO_BYTE_INDEX::GREEN]        = 0x0;
    buffer[AKKO_BYTE_INDEX::BLUE]         = calculateFiller(buffer, AKKO_BYTE_INDEX::COMMAND, AKKO_BYTE_INDEX::GREEN);

    int send_buffer_result = hid_send_feature_report(dev, buffer, AKKO_PACKET_LENGTH_WITH_REPORT);
    if(send_buffer_result<0)
    {
        LOG_ERROR("[%s]: Send Buffer Error. HIDAPI Error: %ls", device_name.c_str(), hid_error(dev));
    }
}

bool AkkoController::HasLightStrips()
{
    return has_light_strips;
}

std::vector<RGBColor> AkkoController::getLedsData(AKKO_ZONE zone)
{
    std::vector<RGBColor> colors;
    if(zone != AKKO_ZONE::Z_KEYBOARD)
    {
        return colors; // Only keyboard supports custom per-LED colors
    }

    colors.reserve(AKKO_ZONE_SIZE::Z_KEYBOARD_SIZE);

    unsigned char buffer[AKKO_PACKET_LENGTH_WITH_REPORT] = { 0x00 };

    int color_byte_k = 0;
    unsigned int current_reconstructed_val = 0;

    int pages = AKKO_ZONE_SIZE::Z_KEYBOARD_SIZE / ((AKKO_PACKET_LENGTH - AKKO_BYTE_INDEX::CUSTOM_FILLER) / 3);
    for(int i = 0; i < pages ; ++i)
    {
        memset(buffer, 0, sizeof(buffer));

        buffer[AKKO_BYTE_INDEX::COMMAND]       = AKKO_BYTE_COMMAND::GET | AKKO_BYTE_COMMAND::CUSTOM_KEYBOARD;
        buffer[AKKO_BYTE_INDEX::MODE]          = 0x00;
        buffer[AKKO_BYTE_INDEX::SPEED]         = i;
        buffer[AKKO_BYTE_INDEX::BRIGHTNESS]    = 0x00;
        buffer[AKKO_BYTE_INDEX::FLAGS]         = 0x00;
        buffer[AKKO_BYTE_INDEX::RED]           = 0x00;
        buffer[AKKO_BYTE_INDEX::GREEN]         = 0x00;
        buffer[AKKO_BYTE_INDEX::CUSTOM_FILLER] = calculateFiller(buffer, AKKO_BYTE_INDEX::COMMAND, AKKO_BYTE_INDEX::GREEN);

        int send_buffer_result = hid_send_feature_report(dev, buffer, AKKO_PACKET_LENGTH_WITH_REPORT);
        if(send_buffer_result < 0)
        {
            LOG_ERROR("[%s]: Send Buffer Error (Get Data). HIDAPI Error: %ls", device_name.c_str(), hid_error(dev));
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        int get_buffer_result = hid_get_feature_report(dev, buffer, AKKO_PACKET_LENGTH_WITH_REPORT);
        if(get_buffer_result < 0)
        {
            LOG_ERROR("[%s]: Get Buffer Error. HIDAPI Error: %ls", device_name.c_str(), hid_error(dev));
            continue;
        }

        int buffer_idx = 1;

        while (buffer_idx < (AKKO_PACKET_LENGTH_WITH_REPORT))
        {
            unsigned char raw_byte = buffer[buffer_idx];

            current_reconstructed_val |= (raw_byte) << (color_byte_k * 8);

            buffer_idx++;
            color_byte_k++;

            if(color_byte_k >= 3)
            {
                colors.push_back(current_reconstructed_val);
                current_reconstructed_val = 0;
                color_byte_k = 0;
            }
        }
    }
    return colors;
}

std::string AkkoController::getZoneName(AKKO_ZONE zone)
{
    if(zone == AKKO_ZONE::Z_KEYBOARD)
    {
        return "Keyboard";
    }
    else if(zone == AKKO_ZONE::Z_LIGHT_STRIPS)
    {
        return "Light Strips";
    }
    return "Unknown Zone";
}

AKKO_BYTE_COMMAND AkkoController::getByteFromZone(AKKO_ZONE zone)
{
    if(zone == AKKO_ZONE::Z_KEYBOARD)
    {
        return AKKO_BYTE_COMMAND::KEYBOARD;
    }
    else if(zone == AKKO_ZONE::Z_LIGHT_STRIPS)
    {
        return AKKO_BYTE_COMMAND::LIGHT_STRIPS;
    }
    return AKKO_BYTE_COMMAND::KEYBOARD;
}


bool IsZoneCompatibleWith(AKKO_ZONE zone, AKKO_MODE mode)
{
    return zone == AKKO_ZONE::Z_KEYBOARD || (AKKO_MODE::OFF <= mode && mode <= AKKO_MODE::RIPPLE);
}
