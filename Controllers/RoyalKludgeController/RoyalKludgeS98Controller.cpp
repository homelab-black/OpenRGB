/*---------------------------------------------------------*\
| RoyalKludgeS98Controller.cpp                              |
|                                                           |
|   Driver for Royal Kludge S98 keyboard                    |
|                                                           |
|   manumyx                                     03 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <chrono>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <thread>
#include "RoyalKludgeS98Controller.h"
#include "LogManager.h"
#include "StringUtils.h"

#define ROYAL_KLUDGE_S98_WIRED_WRITE_DELAY_MS               200
#define ROYAL_KLUDGE_S98_DONGLE_CHUNK_DELAY_MS              15
#define ROYAL_KLUDGE_S98_DONGLE_READ_TIMEOUT_MS             200
#define ROYAL_KLUDGE_S98_HID_MAX_STR                        255

static const unsigned char default_profile_data[ROYAL_KLUDGE_S98_PROFILE_SIZE] =
{
    0x00, 0x03, 0x03, 0x03, 0x00, 0x00, 0x04, 0x04, 0x07, 0x00, 0x0B, 0x20, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x00, 0xFF, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x13, 0x47, 0x14, 0x47, 0x14, 0x47,
    0x14, 0x47, 0x14, 0x47, 0x14, 0x47, 0x14, 0x47, 0x14, 0x47, 0x14, 0x47, 0x14, 0x47, 0x14, 0x47,
    0x14, 0x47, 0x14, 0x47, 0x14, 0x47, 0x14, 0x47, 0x14, 0x47, 0x14, 0x47, 0x14, 0x47, 0x04, 0x47,
    0x07, 0x47, 0x07, 0x47, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44, 0x07, 0x44,
    0x07, 0x44, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x5A, 0xA5
};

static const unsigned char default_effect_row[21] =
{
    0xFF, 0x00, 0xFF,
    0xFF, 0x00, 0x00,
    0x00, 0xFF, 0x00,
    0x00, 0x00, 0xFF,
    0xFF, 0xFF, 0x00,
    0xFF, 0x00, 0xFF,
    0x00, 0xFF, 0xFF
};

RoyalKludgeS98Controller::RoyalKludgeS98Controller(hid_device* dev_handle, const char* path, std::string dev_name, bool is_wireless_mode)
{
    dev                  = dev_handle;
    location             = path;
    name                 = dev_name;
    is_wireless          = is_wireless_mode;
    saved_mode_selection = 0x00;
    saved_mode           = ROYAL_KLUDGE_S98_MODE_NEON_STREAM;

    std::memcpy(profile_data, default_profile_data, ROYAL_KLUDGE_S98_PROFILE_SIZE);
    std::memset(effect_data, 0x00, ROYAL_KLUDGE_S98_EFFECT_TABLE_SIZE);
    std::memset(last_led_buffer, 0x00, ROYAL_KLUDGE_S98_LED_BUFFER_SIZE);
    led_buffer_valid = false;
    std::memset(last_direct_buffer, 0x00, ROYAL_KLUDGE_S98_LED_BUFFER_SIZE);
    direct_buffer_valid = false;

    for(unsigned int mode_idx = 1; mode_idx <= ROYAL_KLUDGE_S98_MODE_BLOSSOMING; mode_idx++)
    {
        std::memcpy(&effect_data[mode_idx * 21], default_effect_row, sizeof(default_effect_row));
    }

    if(!is_wireless)
    {
        ReadPassword();
        ReadProfile();
        ReadEffectTable();
    }
    else
    {
        if(ReadProfile())
        {
            ReadPassword();
        }
    }

    if(profile_data[9] == 0x00 && profile_data[10] >= ROYAL_KLUDGE_S98_MODE_DIRECT)
    {
        saved_mode_selection = 0x00;
        saved_mode           = ROYAL_KLUDGE_S98_MODE_NEON_STREAM;
        profile_data[9]      = saved_mode_selection;
        profile_data[10]     = saved_mode;
        WriteProfile();
    }
    else
    {
        saved_mode_selection = profile_data[9];
        saved_mode           = profile_data[10];
    }
}

RoyalKludgeS98Controller::~RoyalKludgeS98Controller()
{
    RestoreMode();
    hid_close(dev);
}

std::string RoyalKludgeS98Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string RoyalKludgeS98Controller::GetNameString()
{
    return(name);
}

std::string RoyalKludgeS98Controller::GetSerialString()
{
    wchar_t serial_string[ROYAL_KLUDGE_S98_HID_MAX_STR] = { 0 };
    int ret = hid_get_serial_number_string(dev, serial_string, ROYAL_KLUDGE_S98_HID_MAX_STR);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

std::string RoyalKludgeS98Controller::GetFirmwareVersion()
{
    return(version);
}

bool RoyalKludgeS98Controller::IsWireless()
{
    return(is_wireless);
}

unsigned char RoyalKludgeS98Controller::GetCurrentMode()
{
    if(profile_data[9] == 0x01)
    {
        return(ROYAL_KLUDGE_S98_MODE_SELF_DEFINE);
    }

    unsigned char mode = profile_data[10];

    if(mode > ROYAL_KLUDGE_S98_MODE_DIRECT)
    {
        mode = ROYAL_KLUDGE_S98_MODE_NEON_STREAM;
    }

    return(mode);
}

unsigned char RoyalKludgeS98Controller::GetEffectBrightness(unsigned char mode)
{
    if(mode < ROYAL_KLUDGE_S98_MODE_STATIC || mode > ROYAL_KLUDGE_S98_MODE_BLOSSOMING)
    {
        return(ROYAL_KLUDGE_S98_BRIGHTNESS_DEFAULT);
    }

    unsigned int offset = 56 + ((unsigned int)mode * 2);
    unsigned char brightness = profile_data[offset];

    if(brightness > ROYAL_KLUDGE_S98_BRIGHTNESS_MAX)
    {
        brightness = ROYAL_KLUDGE_S98_BRIGHTNESS_MAX;
    }

    return(brightness);
}

unsigned char RoyalKludgeS98Controller::GetEffectSpeed(unsigned char mode)
{
    if(mode < ROYAL_KLUDGE_S98_MODE_STATIC || mode > ROYAL_KLUDGE_S98_MODE_BLOSSOMING)
    {
        return(ROYAL_KLUDGE_S98_SPEED_DEFAULT);
    }

    unsigned int offset = 56 + ((unsigned int)mode * 2);
    unsigned char speed = (profile_data[offset + 1] >> 4) & 0x0F;

    if(speed > ROYAL_KLUDGE_S98_SPEED_MAX)
    {
        speed = ROYAL_KLUDGE_S98_SPEED_MAX;
    }

    return(speed);
}

unsigned char RoyalKludgeS98Controller::GetEffectColorFlag(unsigned char mode)
{
    if(mode < ROYAL_KLUDGE_S98_MODE_STATIC || mode > ROYAL_KLUDGE_S98_MODE_BLOSSOMING)
    {
        return(ROYAL_KLUDGE_S98_COLOR_FLAG_RANDOM);
    }

    unsigned int offset = 56 + ((unsigned int)mode * 2);
    unsigned char color_flag = profile_data[offset + 1] & 0x0F;

    if(color_flag == ROYAL_KLUDGE_S98_COLOR_FLAG_SPECIFIC)
    {
        return(ROYAL_KLUDGE_S98_COLOR_FLAG_SPECIFIC);
    }

    return(ROYAL_KLUDGE_S98_COLOR_FLAG_RANDOM);
}

RGBColor RoyalKludgeS98Controller::GetEffectColor(unsigned char mode)
{
    if(mode < ROYAL_KLUDGE_S98_MODE_STATIC || mode > ROYAL_KLUDGE_S98_MODE_BLOSSOMING)
    {
        return(ToRGBColor(0xFF, 0x00, 0xFF));
    }

    unsigned int offset = (unsigned int)mode * 21;
    return(ToRGBColor(effect_data[offset], effect_data[offset + 1], effect_data[offset + 2]));
}

void RoyalKludgeS98Controller::SetMode
    (
    unsigned char   mode,
    unsigned char   brightness,
    unsigned char   speed,
    unsigned char   color_flag,
    RGBColor        color
    )
{
    std::lock_guard<std::mutex> lock(io_mutex);

    if(is_wireless && !IsDongleKeyboardOnline())
    {
        LOG_DIALOG("Royal Kludge S98 (2.4GHz) is not responding (the keyboard may be asleep or switched to wired mode). Wake the keyboard and try again.");
        return;
    }

    if(mode == ROYAL_KLUDGE_S98_MODE_DIRECT)
    {
        direct_buffer_valid = false;

        if(!is_wireless)
        {
            if(profile_data[9] != 0x00 || profile_data[10] != ROYAL_KLUDGE_S98_MODE_DIRECT)
            {
                profile_data[9]  = 0x00;
                profile_data[10] = ROYAL_KLUDGE_S98_MODE_DIRECT;
                WriteProfile();
            }
        }
        else
        {
            profile_data[9]  = 0x01;
            profile_data[10] = ROYAL_KLUDGE_S98_MODE_SELF_DEFINE;
            WriteProfile();
        }
        return;
    }

    if(mode == ROYAL_KLUDGE_S98_MODE_SELF_DEFINE)
    {
        profile_data[9]      = 0x01;
        profile_data[10]     = ROYAL_KLUDGE_S98_MODE_SELF_DEFINE;
        saved_mode_selection = 0x01;
        saved_mode           = ROYAL_KLUDGE_S98_MODE_SELF_DEFINE;
        led_buffer_valid     = false;
        WriteProfile();
        return;
    }

    profile_data[9]      = 0x00;
    profile_data[10]     = mode;
    saved_mode_selection = 0x00;
    saved_mode           = mode;

    if(mode >= ROYAL_KLUDGE_S98_MODE_STATIC && mode <= ROYAL_KLUDGE_S98_MODE_BLOSSOMING)
    {
        if(brightness > ROYAL_KLUDGE_S98_BRIGHTNESS_MAX)
        {
            brightness = ROYAL_KLUDGE_S98_BRIGHTNESS_MAX;
        }

        if(speed > ROYAL_KLUDGE_S98_SPEED_MAX)
        {
            speed = ROYAL_KLUDGE_S98_SPEED_MAX;
        }

        unsigned int profile_offset = 56 + ((unsigned int)mode * 2);
        profile_data[profile_offset]     = brightness;
        profile_data[profile_offset + 1] = ((speed & 0x0F) << 4) | (color_flag & 0x0F);

        if(color_flag == ROYAL_KLUDGE_S98_COLOR_FLAG_SPECIFIC)
        {
            unsigned int effect_offset = (unsigned int)mode * 21;
            effect_data[effect_offset + 0] = RGBGetRValue(color);
            effect_data[effect_offset + 1] = RGBGetGValue(color);
            effect_data[effect_offset + 2] = RGBGetBValue(color);
            WriteEffectTable();
        }
    }

    WriteProfile();
}

void RoyalKludgeS98Controller::SetLedColors(const std::vector<RoyalKludgeS98LedSlot>& led_slots)
{
    std::lock_guard<std::mutex> lock(io_mutex);

    if(is_wireless && !IsDongleKeyboardOnline())
    {
        LOG_DIALOG("Royal Kludge S98 (2.4GHz) is not responding (the keyboard may be asleep or switched to wired mode). Wake the keyboard and try again.");
        return;
    }

    bool mode_changed = false;

    if(profile_data[9] != 0x01 || profile_data[10] != ROYAL_KLUDGE_S98_MODE_SELF_DEFINE)
    {
        profile_data[9]      = 0x01;
        profile_data[10]     = ROYAL_KLUDGE_S98_MODE_SELF_DEFINE;
        saved_mode_selection = 0x01;
        saved_mode           = ROYAL_KLUDGE_S98_MODE_SELF_DEFINE;
        WriteProfile();
        mode_changed = true;
    }

    unsigned char led_buffer[ROYAL_KLUDGE_S98_LED_BUFFER_SIZE];
    std::memset(led_buffer, 0x00, sizeof(led_buffer));

    for(unsigned int i = 0; i < led_slots.size(); i++)
    {
        unsigned char idx = led_slots[i].matrix_index;

        if(idx < ROYAL_KLUDGE_S98_MATRIX_CELL_COUNT)
        {
            led_buffer[idx]                                                = RGBGetRValue(led_slots[i].color);
            led_buffer[idx + ROYAL_KLUDGE_S98_MATRIX_CELL_COUNT]           = RGBGetGValue(led_slots[i].color);
            led_buffer[idx + (ROYAL_KLUDGE_S98_MATRIX_CELL_COUNT * 2)]     = RGBGetBValue(led_slots[i].color);
        }
    }

    if(!mode_changed && led_buffer_valid && std::memcmp(led_buffer, last_led_buffer, sizeof(led_buffer)) == 0)
    {
        return;
    }

    if(WriteLedBuffer(led_buffer))
    {
        std::memcpy(last_led_buffer, led_buffer, sizeof(led_buffer));
        led_buffer_valid = true;
    }
}

void RoyalKludgeS98Controller::SetLedsDirect(const std::vector<RoyalKludgeS98LedSlot>& led_slots)
{
    if(is_wireless)
    {
        SetLedColors(led_slots);
        return;
    }

    std::lock_guard<std::mutex> lock(io_mutex);

    bool mode_changed = false;

    if(profile_data[9] != 0x00 || profile_data[10] != ROYAL_KLUDGE_S98_MODE_DIRECT)
    {
        profile_data[9]  = 0x00;
        profile_data[10] = ROYAL_KLUDGE_S98_MODE_DIRECT;
        WriteProfile();
        mode_changed = true;
    }

    unsigned char direct_buffer[ROYAL_KLUDGE_S98_LED_BUFFER_SIZE];
    std::memset(direct_buffer, 0x00, sizeof(direct_buffer));

    for(unsigned int i = 0; i < led_slots.size(); i++)
    {
        unsigned int idx = led_slots[i].matrix_index;

        if(idx < ROYAL_KLUDGE_S98_MATRIX_CELL_COUNT)
        {
            unsigned int offset       = idx * 3;
            direct_buffer[offset + 0] = RGBGetGValue(led_slots[i].color);
            direct_buffer[offset + 1] = RGBGetRValue(led_slots[i].color);
            direct_buffer[offset + 2] = RGBGetBValue(led_slots[i].color);
        }
    }

    /*-----------------------------------------------------*\
    | Skip the USB transfer if the frame is unchanged       |
    \*-----------------------------------------------------*/
    if(!mode_changed && direct_buffer_valid && std::memcmp(direct_buffer, last_direct_buffer, sizeof(direct_buffer)) == 0)
    {
        return;
    }

    if(SendWiredDirectReport(direct_buffer, ROYAL_KLUDGE_S98_LED_BUFFER_SIZE))
    {
        std::memcpy(last_direct_buffer, direct_buffer, sizeof(direct_buffer));
        direct_buffer_valid = true;
    }
}

void RoyalKludgeS98Controller::RestoreMode()
{
    std::lock_guard<std::mutex> lock(io_mutex);

    if(profile_data[9] == 0x00 && profile_data[10] == ROYAL_KLUDGE_S98_MODE_DIRECT)
    {
        profile_data[9]     = saved_mode_selection;
        profile_data[10]    = saved_mode;
        direct_buffer_valid = false;
        WriteProfile();
    }
}

unsigned char RoyalKludgeS98Controller::CalculateDongleChecksum(const unsigned char* packet)
{
    unsigned int sum = 0;

    for(unsigned int i = 0; i < (ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE - 1); i++)
    {
        sum += packet[i];
    }

    return((unsigned char)(sum & 0xFF));
}

bool RoyalKludgeS98Controller::IsS98Password(unsigned int password)
{
    switch(password)
    {
        case 0x06000023: /* RK-S98 US          */
        case 0x0600002C: /* RK-S98 ES (Purple) */
        case 0x0600006B: /* RK-S98 POR         */
        case 0x0600006D: /* RK-S98 TH          */
        case 0x0600006E: /* RK-S98 DE          */
        case 0x06000072: /* RK-S98 ES          */
        case 0x06000073: /* RK-S98 JP          */
        case 0x06000074: /* RK-S98 RU          */
        case 0x06000077: /* RK-S98 TC          */
        case 0x06000078: /* RK-S98 KR          */
        case 0x06000079: /* RK-S98 FR          */
            return(true);

        default:
            return(false);
    }
}

bool RoyalKludgeS98Controller::ProbeDongleS98(hid_device* dev_handle)
{
    unsigned char packet[ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE];
    std::memset(packet, 0x00, sizeof(packet));

    packet[0]  = ROYAL_KLUDGE_S98_DONGLE_REPORT_ID;
    packet[1]  = ROYAL_KLUDGE_S98_CMD_DONGLE_GET_STATUS;
    packet[2]  = 0x01;
    packet[19] = CalculateDongleChecksum(packet);

    if(hid_write(dev_handle, packet, ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE) < 0)
    {
        return(false);
    }

    bool status_ok       = false;
    bool keyboard_online = false;

    for(unsigned int attempt = 0; attempt < 5; attempt++)
    {
        unsigned char response[64];
        std::memset(response, 0x00, sizeof(response));

        int bytes_read = hid_read_timeout(dev_handle, response, ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE, ROYAL_KLUDGE_S98_DONGLE_READ_TIMEOUT_MS);

        if(bytes_read <= 0)
        {
            break;
        }

        if(response[0] == ROYAL_KLUDGE_S98_DONGLE_REPORT_ID && response[1] == ROYAL_KLUDGE_S98_CMD_DONGLE_GET_STATUS)
        {
            status_ok       = true;
            keyboard_online = (response[5] > 0);
            break;
        }
    }

    if(!status_ok)
    {
        return(false);
    }

    if(!keyboard_online)
    {
        return(true);
    }

    std::memset(packet, 0x00, sizeof(packet));
    packet[0]  = ROYAL_KLUDGE_S98_DONGLE_REPORT_ID;
    packet[1]  = ROYAL_KLUDGE_S98_CMD_DONGLE_GET_PASSWORD;
    packet[2]  = 0x01;
    packet[19] = CalculateDongleChecksum(packet);

    if(hid_write(dev_handle, packet, ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE) < 0)
    {
        return(true);
    }

    for(unsigned int attempt = 0; attempt < 5; attempt++)
    {
        unsigned char response[64];
        std::memset(response, 0x00, sizeof(response));

        int bytes_read = hid_read_timeout(dev_handle, response, ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE, ROYAL_KLUDGE_S98_DONGLE_READ_TIMEOUT_MS);

        if(bytes_read <= 0)
        {
            break;
        }

        if(response[0] == ROYAL_KLUDGE_S98_DONGLE_REPORT_ID && response[1] == ROYAL_KLUDGE_S98_CMD_DONGLE_GET_PASSWORD)
        {
            unsigned int high_part = ((unsigned int)response[5] << 24)
                                   | ((unsigned int)response[6] << 16)
                                   | ((unsigned int)response[7] << 8)
                                   | ((unsigned int)response[8]);
            unsigned int low_part  = ((unsigned int)response[9] << 8)
                                   | ((unsigned int)response[10]);
            unsigned int password  = high_part + low_part;

            return(IsS98Password(password));
        }
    }

    return(true);
}

bool RoyalKludgeS98Controller::IsDongleKeyboardOnline()
{
    unsigned char packet[ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE];
    std::memset(packet, 0x00, sizeof(packet));

    packet[0]  = ROYAL_KLUDGE_S98_DONGLE_REPORT_ID;
    packet[1]  = ROYAL_KLUDGE_S98_CMD_DONGLE_GET_STATUS;
    packet[2]  = 0x01;
    packet[19] = CalculateDongleChecksum(packet);

    if(hid_write(dev, packet, ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE) < 0)
    {
        return(false);
    }

    for(unsigned int attempt = 0; attempt < 5; attempt++)
    {
        unsigned char response[64];
        std::memset(response, 0x00, sizeof(response));

        int bytes_read = hid_read_timeout(dev, response, ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE, ROYAL_KLUDGE_S98_DONGLE_READ_TIMEOUT_MS);

        if(bytes_read <= 0)
        {
            break;
        }

        if(response[0] == ROYAL_KLUDGE_S98_DONGLE_REPORT_ID && response[1] == ROYAL_KLUDGE_S98_CMD_DONGLE_GET_STATUS)
        {
            return(response[5] > 0);
        }
    }

    return(false);
}

bool RoyalKludgeS98Controller::ReadPassword()
{
    if(is_wireless)
    {
        unsigned char packet[ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE];
        std::memset(packet, 0x00, sizeof(packet));

        packet[0]  = ROYAL_KLUDGE_S98_DONGLE_REPORT_ID;
        packet[1]  = ROYAL_KLUDGE_S98_CMD_DONGLE_GET_PASSWORD;
        packet[2]  = 0x01;
        packet[19] = CalculateDongleChecksum(packet);

        if(hid_write(dev, packet, ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE) < 0)
        {
            return(false);
        }

        for(unsigned int attempt = 0; attempt < 5; attempt++)
        {
            unsigned char response[64];
            std::memset(response, 0x00, sizeof(response));

            int bytes_read = hid_read_timeout(dev, response, ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE, ROYAL_KLUDGE_S98_DONGLE_READ_TIMEOUT_MS);

            if(bytes_read <= 0)
            {
                break;
            }

            if(response[0] == ROYAL_KLUDGE_S98_DONGLE_REPORT_ID && response[1] == ROYAL_KLUDGE_S98_CMD_DONGLE_GET_PASSWORD)
            {
                std::ostringstream stream;
                stream << std::hex << std::uppercase << std::setfill('0')
                       << std::setw(2) << (unsigned int)response[14]
                       << std::setw(2) << (unsigned int)response[13];
                version = stream.str();
                return(true);
            }
        }

        return(false);
    }
    else
    {
        unsigned char payload[10];
        std::memset(payload, 0x00, sizeof(payload));

        if(!GetWiredReport(ROYAL_KLUDGE_S98_CMD_WIRED_GET_PASSWORD, payload, sizeof(payload)))
        {
            return(false);
        }

        std::ostringstream stream;
        stream << std::hex << std::uppercase << std::setfill('0')
               << std::setw(2) << (unsigned int)payload[9]
               << std::setw(2) << (unsigned int)payload[8];
        version = stream.str();
        return(true);
    }
}

bool RoyalKludgeS98Controller::ReadProfile()
{
    unsigned char temp_profile[ROYAL_KLUDGE_S98_PROFILE_SIZE];
    std::memset(temp_profile, 0x00, sizeof(temp_profile));

    if(is_wireless)
    {
        unsigned char packet[ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE];
        std::memset(packet, 0x00, sizeof(packet));

        packet[0]  = ROYAL_KLUDGE_S98_DONGLE_REPORT_ID;
        packet[1]  = ROYAL_KLUDGE_S98_CMD_DONGLE_GET_STATUS;
        packet[2]  = 0x01;
        packet[19] = CalculateDongleChecksum(packet);

        if(hid_write(dev, packet, ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE) < 0)
        {
            return(false);
        }

        bool keyboard_online = false;

        for(unsigned int attempt = 0; attempt < 5; attempt++)
        {
            unsigned char response[64];
            std::memset(response, 0x00, sizeof(response));

            int bytes_read = hid_read_timeout(dev, response, ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE, ROYAL_KLUDGE_S98_DONGLE_READ_TIMEOUT_MS);

            if(bytes_read <= 0)
            {
                break;
            }

            if(response[0] == ROYAL_KLUDGE_S98_DONGLE_REPORT_ID && response[1] == ROYAL_KLUDGE_S98_CMD_DONGLE_GET_STATUS)
            {
                keyboard_online = (response[5] > 0);
                break;
            }
        }

        if(!keyboard_online)
        {
            return(false);
        }

        std::memset(packet, 0x00, sizeof(packet));
        packet[0]  = ROYAL_KLUDGE_S98_DONGLE_REPORT_ID;
        packet[1]  = ROYAL_KLUDGE_S98_CMD_DONGLE_GET_PROFILE;
        packet[2]  = 0x01;
        packet[19] = CalculateDongleChecksum(packet);

        if(hid_write(dev, packet, ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE) < 0)
        {
            return(false);
        }

        unsigned int offset = 0;

        for(unsigned int attempt = 0; attempt < 24; attempt++)
        {
            unsigned char response[64];
            std::memset(response, 0x00, sizeof(response));

            int bytes_read = hid_read_timeout(dev, response, ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE, ROYAL_KLUDGE_S98_DONGLE_READ_TIMEOUT_MS);

            if(bytes_read <= 0)
            {
                break;
            }

            if(response[0] == ROYAL_KLUDGE_S98_DONGLE_REPORT_ID && response[1] == ROYAL_KLUDGE_S98_CMD_DONGLE_GET_PROFILE)
            {
                unsigned int chunk_len = response[4] & 0x0F;

                if((offset + chunk_len) <= ROYAL_KLUDGE_S98_PROFILE_SIZE)
                {
                    std::memcpy(&temp_profile[offset], &response[5], chunk_len);
                    offset += chunk_len;
                }

                if(offset >= ROYAL_KLUDGE_S98_PROFILE_SIZE)
                {
                    break;
                }
            }
        }
    }
    else
    {
        if(!GetWiredReport(ROYAL_KLUDGE_S98_CMD_WIRED_GET_PROFILE, temp_profile, ROYAL_KLUDGE_S98_PROFILE_SIZE))
        {
            return(false);
        }
    }

    if(temp_profile[126] == 0x5A && temp_profile[127] == 0xA5)
    {
        std::memcpy(profile_data, temp_profile, ROYAL_KLUDGE_S98_PROFILE_SIZE);
        return(true);
    }

    return(false);
}

bool RoyalKludgeS98Controller::ReadEffectTable()
{
    unsigned char temp_effect[ROYAL_KLUDGE_S98_EFFECT_TABLE_SIZE];
    std::memset(temp_effect, 0x00, sizeof(temp_effect));

    if(!GetWiredReport(ROYAL_KLUDGE_S98_CMD_WIRED_GET_LED_EFFECT, temp_effect, ROYAL_KLUDGE_S98_EFFECT_TABLE_SIZE))
    {
        return(false);
    }

    std::memcpy(effect_data, temp_effect, ROYAL_KLUDGE_S98_EFFECT_TABLE_SIZE);
    return(true);
}

bool RoyalKludgeS98Controller::WriteProfile()
{
    profile_data[126] = 0x5A;
    profile_data[127] = 0xA5;

    if(is_wireless)
    {
        return(SendDongleChunked(ROYAL_KLUDGE_S98_CMD_SET_PROFILE, profile_data, ROYAL_KLUDGE_S98_PROFILE_SIZE));
    }
    else
    {
        return(SendWiredReport(ROYAL_KLUDGE_S98_CMD_SET_PROFILE, profile_data, ROYAL_KLUDGE_S98_PROFILE_SIZE));
    }
}

bool RoyalKludgeS98Controller::WriteEffectTable()
{
    if(is_wireless)
    {
        return(SendDongleChunked(ROYAL_KLUDGE_S98_CMD_DONGLE_SET_LED_EFFECT, effect_data, ROYAL_KLUDGE_S98_EFFECT_TABLE_SIZE));
    }
    else
    {
        return(SendWiredReport(ROYAL_KLUDGE_S98_CMD_WIRED_SET_LED_EFFECT, effect_data, ROYAL_KLUDGE_S98_EFFECT_TABLE_SIZE));
    }
}

bool RoyalKludgeS98Controller::WriteLedBuffer(const unsigned char* led_buffer)
{
    if(is_wireless)
    {
        return(SendDongleChunked(ROYAL_KLUDGE_S98_CMD_DONGLE_SET_LED_COLORS, led_buffer, ROYAL_KLUDGE_S98_LED_BUFFER_SIZE));
    }
    else
    {
        return(SendWiredReport(ROYAL_KLUDGE_S98_CMD_WIRED_SET_LED_COLORS, led_buffer, ROYAL_KLUDGE_S98_LED_BUFFER_SIZE));
    }
}

bool RoyalKludgeS98Controller::SendWiredReport(unsigned char cmd_id, const unsigned char* payload, unsigned int length)
{
    unsigned char buffer[ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE];
    std::memset(buffer, 0x00, sizeof(buffer));

    buffer[0] = ROYAL_KLUDGE_S98_WIRED_REPORT_ID;
    buffer[1] = cmd_id;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    buffer[4] = 0x01;
    buffer[5] = 0x00;
    buffer[6] = (unsigned char)(length & 0xFF);
    buffer[7] = (unsigned char)((length >> 8) & 0xFF);

    unsigned int copy_len = length;

    if(copy_len > (ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE - ROYAL_KLUDGE_S98_WIRED_HEADER_SIZE))
    {
        copy_len = ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE - ROYAL_KLUDGE_S98_WIRED_HEADER_SIZE;
    }

    std::memcpy(&buffer[ROYAL_KLUDGE_S98_WIRED_HEADER_SIZE], payload, copy_len);

    int ret = hid_send_feature_report(dev, buffer, ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE);

    std::this_thread::sleep_for(std::chrono::milliseconds(ROYAL_KLUDGE_S98_WIRED_WRITE_DELAY_MS));

    return(ret >= 0);
}

bool RoyalKludgeS98Controller::SendWiredDirectReport(const unsigned char* payload, unsigned int length)
{
    unsigned char buffer[ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE];
    std::memset(buffer, 0x00, sizeof(buffer));

    buffer[0] = ROYAL_KLUDGE_S98_WIRED_REPORT_ID;
    buffer[1] = ROYAL_KLUDGE_S98_CMD_WIRED_SET_LEDS_DIRECT;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    buffer[4] = 0x01;
    buffer[5] = 0x00;
    buffer[6] = (unsigned char)(length & 0xFF);
    buffer[7] = (unsigned char)((length >> 8) & 0xFF);

    unsigned int copy_len = length;

    if(copy_len > (ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE - ROYAL_KLUDGE_S98_WIRED_HEADER_SIZE))
    {
        copy_len = ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE - ROYAL_KLUDGE_S98_WIRED_HEADER_SIZE;
    }

    std::memcpy(&buffer[ROYAL_KLUDGE_S98_WIRED_HEADER_SIZE], payload, copy_len);

    int ret = hid_send_feature_report(dev, buffer, ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE);

    return(ret >= 0);
}

bool RoyalKludgeS98Controller::GetWiredReport(unsigned char cmd_id, unsigned char* payload, unsigned int length)
{
    unsigned char request[ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE];
    std::memset(request, 0x00, sizeof(request));

    request[0] = ROYAL_KLUDGE_S98_WIRED_REPORT_ID;
    request[1] = cmd_id;
    request[2] = 0x00;
    request[3] = 0x00;
    request[4] = 0x01;
    request[5] = 0x00;
    request[6] = (unsigned char)(length & 0xFF);
    request[7] = (unsigned char)((length >> 8) & 0xFF);

    if(hid_send_feature_report(dev, request, ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE) < 0)
    {
        return(false);
    }

    unsigned char response[ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE];
    std::memset(response, 0x00, sizeof(response));
    response[0] = ROYAL_KLUDGE_S98_WIRED_REPORT_ID;

    if(hid_get_feature_report(dev, response, ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE) < 0)
    {
        return(false);
    }

    if(response[1] != cmd_id)
    {
        return(false);
    }

    unsigned int copy_len = length;

    if(copy_len > (ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE - ROYAL_KLUDGE_S98_WIRED_HEADER_SIZE))
    {
        copy_len = ROYAL_KLUDGE_S98_WIRED_REPORT_SIZE - ROYAL_KLUDGE_S98_WIRED_HEADER_SIZE;
    }

    std::memcpy(payload, &response[ROYAL_KLUDGE_S98_WIRED_HEADER_SIZE], copy_len);
    return(true);
}

bool RoyalKludgeS98Controller::SendDongleChunked(unsigned char cmd_id, const unsigned char* payload, unsigned int length)
{
    unsigned int pkg_num = (length + ROYAL_KLUDGE_S98_DONGLE_CHUNK_SIZE - 1) / ROYAL_KLUDGE_S98_DONGLE_CHUNK_SIZE;

    for(unsigned int pkg_idx = 0; pkg_idx < pkg_num; pkg_idx++)
    {
        unsigned int offset    = pkg_idx * ROYAL_KLUDGE_S98_DONGLE_CHUNK_SIZE;
        unsigned int chunk_len = length - offset;

        if(chunk_len > ROYAL_KLUDGE_S98_DONGLE_CHUNK_SIZE)
        {
            chunk_len = ROYAL_KLUDGE_S98_DONGLE_CHUNK_SIZE;
        }

        unsigned char packet[ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE];
        std::memset(packet, 0x00, sizeof(packet));

        packet[0] = ROYAL_KLUDGE_S98_DONGLE_REPORT_ID;
        packet[1] = cmd_id;
        packet[2] = (unsigned char)(pkg_num & 0x7F);
        packet[3] = (unsigned char)(pkg_idx & 0x7F);
        packet[4] = (unsigned char)(chunk_len & 0x0F);

        std::memcpy(&packet[5], &payload[offset], chunk_len);

        packet[19] = CalculateDongleChecksum(packet);

        if(hid_write(dev, packet, ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE) < 0)
        {
            return(false);
        }

        for(unsigned int attempt = 0; attempt < 5; attempt++)
        {
            unsigned char response[64];
            std::memset(response, 0x00, sizeof(response));

            int bytes_read = hid_read_timeout(dev, response, ROYAL_KLUDGE_S98_DONGLE_REPORT_SIZE, ROYAL_KLUDGE_S98_DONGLE_READ_TIMEOUT_MS);

            if(bytes_read <= 0)
            {
                break;
            }

            if(response[0] == ROYAL_KLUDGE_S98_DONGLE_REPORT_ID && response[1] == cmd_id)
            {
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(ROYAL_KLUDGE_S98_DONGLE_CHUNK_DELAY_MS));
    }

    return(true);
}
