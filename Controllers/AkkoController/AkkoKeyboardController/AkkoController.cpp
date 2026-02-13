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
#include "LogManager.h"
#include "StringUtils.h"

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
        speed = (AKKO_SPEED)(AKKO_SPEED::S_MAX - speed); // Invert speed (0 is max speed, 4 is min speed)
    }

    unsigned char buffer[AKKO_PACKET_LENGTH + 1] = { 0x00 };
    buffer[AKKO_BYTE_INDEX::COMMAND]      = AKKO_BYTE_COMMAND::SET | command;
    buffer[AKKO_BYTE_INDEX::MODE]         = mode;
    buffer[AKKO_BYTE_INDEX::SPEED]        = speed;
    buffer[AKKO_BYTE_INDEX::BRIGHTNESS]   = brightness;
    buffer[AKKO_BYTE_INDEX::FLAGS]        = flags;
    buffer[AKKO_BYTE_INDEX::RED]          = RGBGetRValue(color);
    buffer[AKKO_BYTE_INDEX::GREEN]        = RGBGetGValue(color);
    buffer[AKKO_BYTE_INDEX::BLUE]         = RGBGetBValue(color);
    buffer[AKKO_BYTE_INDEX::EFFECT_FILLER]   = calculateFiller(buffer, AKKO_BYTE_INDEX::COMMAND, AKKO_BYTE_INDEX::BLUE);

    int send_buffer_result = hid_send_feature_report(dev, buffer, (sizeof(buffer) / sizeof(buffer[0])));
    if(send_buffer_result<0)
    {
        LOG_ERROR("[AKKO]: Send Buffer Error. HIDAPI Error: %ls", hid_error(dev));
    }
}

AkkoZoneInfo AkkoController::GetZoneInfo(AKKO_ZONE zone)
{
    AkkoZoneInfo effect;

    AKKO_BYTE_COMMAND command = getByteFromZone(zone);
    unsigned char buffer[AKKO_PACKET_LENGTH + 1] = { 0x00 };
    buffer[AKKO_BYTE_INDEX::COMMAND]    = AKKO_BYTE_COMMAND::GET | command;
    buffer[AKKO_BYTE_INDEX::MODE]       = 0x00;
    buffer[AKKO_BYTE_INDEX::SPEED]      = 0x00;
    buffer[AKKO_BYTE_INDEX::BRIGHTNESS] = 0x00;
    buffer[AKKO_BYTE_INDEX::FLAGS]      = 0x00;
    buffer[AKKO_BYTE_INDEX::RED]        = 0x00;
    buffer[AKKO_BYTE_INDEX::GREEN]      = 0x00;
    buffer[AKKO_BYTE_INDEX::CUSTOM_FILLER]       = calculateFiller(buffer, AKKO_BYTE_INDEX::COMMAND, AKKO_BYTE_INDEX::GREEN);

    int send_buffer_result = hid_send_feature_report(dev, buffer, (sizeof(buffer) / sizeof(buffer[0])));
    if(send_buffer_result<0)
    {
        LOG_ERROR("[AKKO]: Get Buffer Error. HIDAPI Error: %ls", hid_error(dev));
        return effect;
    }

    memset(buffer, 0, sizeof(buffer));
    int get_buffer_result = hid_get_feature_report(dev, buffer, (sizeof(buffer) / sizeof(buffer[0])));
    if(get_buffer_result<0)
    {
        LOG_ERROR("[AKKO]: Get Buffer Error. HIDAPI Error: %ls", hid_error(dev));
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
    unsigned char buffer[AKKO_PACKET_LENGTH + 1] = { 0x00 };

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

        while (buffer_idx < (AKKO_PACKET_LENGTH + 1) && led_index < zone_info.colors.size())
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

        int send_buffer_result = hid_send_feature_report(dev, buffer, sizeof(buffer));
        if(send_buffer_result < 0)
        {
            LOG_ERROR("[AKKO]: Send Buffer Error. HIDAPI Error: %ls", hid_error(dev));
        }
    }
}

void AkkoController::SetDirectMusic(const AkkoZoneInfo& zone_info)
{
    if(zone_info.zone != AKKO_ZONE::Z_KEYBOARD || (zone_info.mode != AKKO_MODE::MUSIC_FOLLOW && zone_info.mode != AKKO_MODE::MUSIC_FOLLOW_2))
    {
        return; // Only keyboard supports direct music mode and only if the mode is MUSIC_FOLLOW or MUSIC_FOLLOW_2
    }
    unsigned char buffer[AKKO_PACKET_LENGTH + 1] = { 0x00 };

    buffer[AKKO_BYTE_INDEX::COMMAND]      = AKKO_BYTE_COMMAND::SET | AKKO_BYTE_COMMAND::DIRECT_MUSIC;
    buffer[AKKO_BYTE_INDEX::MODE]         = 0x0;
    buffer[AKKO_BYTE_INDEX::SPEED]        = 0x0;
    buffer[AKKO_BYTE_INDEX::BRIGHTNESS]   = 0x0;
    buffer[AKKO_BYTE_INDEX::FLAGS]        = 0x0;
    buffer[AKKO_BYTE_INDEX::RED]          = 0x0;
    buffer[AKKO_BYTE_INDEX::GREEN]        = 0x0;
    buffer[AKKO_BYTE_INDEX::BLUE]         = calculateFiller(buffer, AKKO_BYTE_INDEX::COMMAND, AKKO_BYTE_INDEX::GREEN);

    for(int i=AKKO_BYTE_INDEX::EFFECT_FILLER; i<AKKO_PACKET_LENGTH + 1; i++)
    {
        buffer[i] = rand() % 7; // TODO: This is just a placeholder, as the actual mapping should be an Array of 32byte From 0 to 6 that maps from 473hz to 1540hz
    }

    int send_buffer_result = hid_send_feature_report(dev, buffer, (sizeof(buffer) / sizeof(buffer[0])));
    if(send_buffer_result<0)
    {
        LOG_ERROR("[AKKO]: Send Buffer Error. HIDAPI Error: %ls", hid_error(dev));
    }
}

void AkkoController::SetDirectColor(const AkkoZoneInfo& zone_info)
{
    if(zone_info.zone != AKKO_ZONE::Z_KEYBOARD || zone_info.mode != AKKO_MODE::LIGHT_SHADOW)
    {
        return; // Only keyboard supports direct color mode, and only in LIGHT_SHADOW mode
    }
    unsigned char buffer[AKKO_PACKET_LENGTH + 1] = { 0x00 };

    buffer[AKKO_BYTE_INDEX::COMMAND]      = AKKO_BYTE_COMMAND::SET | AKKO_BYTE_COMMAND::DIRECT_COLOR;
    buffer[AKKO_BYTE_INDEX::MODE]         = RGBGetRValue(zone_info.color);
    buffer[AKKO_BYTE_INDEX::SPEED]        = RGBGetGValue(zone_info.color);
    buffer[AKKO_BYTE_INDEX::BRIGHTNESS]   = RGBGetBValue(zone_info.color);
    buffer[AKKO_BYTE_INDEX::FLAGS]        = 0x0;
    buffer[AKKO_BYTE_INDEX::RED]          = 0x0;
    buffer[AKKO_BYTE_INDEX::GREEN]        = 0x0;
    buffer[AKKO_BYTE_INDEX::BLUE]         = calculateFiller(buffer, AKKO_BYTE_INDEX::COMMAND, AKKO_BYTE_INDEX::GREEN);

    int send_buffer_result = hid_send_feature_report(dev, buffer, (sizeof(buffer) / sizeof(buffer[0])));
    if(send_buffer_result<0)
    {
        LOG_ERROR("[AKKO]: Send Buffer Error. HIDAPI Error: %ls", hid_error(dev));
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

    unsigned char buffer[AKKO_PACKET_LENGTH + 1] = { 0x00 };

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

        int send_buffer_result = hid_send_feature_report(dev, buffer, sizeof(buffer));
        if(send_buffer_result < 0)
        {
            LOG_ERROR("[AKKO]: Send Buffer Error (Get Data). HIDAPI Error: %ls", hid_error(dev));
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        int get_buffer_result = hid_get_feature_report(dev, buffer, sizeof(buffer));
        if(get_buffer_result < 0)
        {
            LOG_ERROR("[AKKO]: Get Buffer Error. HIDAPI Error: %ls", hid_error(dev));
            continue;
        }

        int buffer_idx = 1;

        while (buffer_idx < (AKKO_PACKET_LENGTH + 1))
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
