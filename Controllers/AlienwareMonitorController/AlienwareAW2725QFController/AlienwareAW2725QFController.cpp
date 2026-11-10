/*---------------------------------------------------------*\
| AlienwareAW2725QFController.cpp                           |
|                                                           |
|   Driver for the Alienware AW2725QF monitor               |
|                                                           |
|   Ferréol DUBOIS COLI (Fefe_du_973)           23 Jan 2025 |
|   Jingcheng Tang (at1289)                     08 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <bitset>
#include <chrono>
#include <cstring>
#include <thread>

#include "AlienwareAW2725QFController.h"
#include "LogManager.h"
#include "StringUtils.h"

/*---------------------------------------------------------*\
| Based on AW3423DWF controller, both monitors use similar  |
| E1 challenge-response authentication protocol. This model |
| separated to implemented full effects support. We could   |
| probably combine both controller in the future after test.|
\*---------------------------------------------------------*/
AlienwareAW2725QFController::AlienwareAW2725QFController(hid_device *dev_handle, const char *path) : dev(dev_handle), location(path)
{
}

bool AlienwareAW2725QFController::SendControlPacket(const unsigned char *data, size_t length)
{
    unsigned char buffer[193] = {0x00};
    if(length != 192)
    {
        return false;
    }
    memcpy(buffer + 1, data, length);

    int result = hid_write(dev, buffer, sizeof(buffer));
    if(result != (int)sizeof(buffer))
    {
        LOG_ERROR("[Alienware AW2725QF] Failed to write HID report (%d bytes)", result);
        return false;
    }
    return true;
}

std::vector<unsigned char> AlienwareAW2725QFController::GetReportResponse()
{
    uint8_t buffer[193];

    memset(buffer, 0, 193);

    if(hid_get_input_report(dev, buffer, sizeof(buffer)) < 17)
    {
        LOG_ERROR("[Alienware AW2725QF] Failed to read authentication challenge");
        return {};
    }

    return std::vector<unsigned char>(buffer + 1, buffer + 17);
}

bool AlienwareAW2725QFController::PerformLogin()
{
    unsigned char init_packet[192] = {0x00};
    init_packet[0] = 0x40;
    init_packet[1] = 0xE1;
    init_packet[2] = 0x01;
    if(!SendControlPacket(init_packet, sizeof(init_packet)))
    {
        return false;
    }

    std::vector<unsigned char> response = GetReportResponse();

    if(response.size() != 16)
    {
        return false;
    }

    std::vector<unsigned char> key = GenerateKey(response);

    unsigned char login_packet[192] = {0x00};
    login_packet[0] = 0x40;
    login_packet[1] = 0xE1;
    login_packet[2] = 0x02;
    memcpy(login_packet + 64, key.data(), key.size());

    if(!SendControlPacket(login_packet, sizeof(login_packet)))
    {
        return false;
    }

    return true;
}

bool AlienwareAW2725QFController::SendDDCCommand(const unsigned char* data, size_t length)
{
    /*-----------------------------------------------------*\
    | Every command sent need a authentication prior,       |
    | sending multiple commands cause commands to fail.     |
    \*-----------------------------------------------------*/
    if(length > 12 || !PerformLogin())
    {
        return false;
    }

    unsigned char packet[192] = {0x00};

    packet[0]       = 0x40;
    packet[1]       = 0xC6;
    packet[6]       = (unsigned char)(length + 3);
    packet[8]       = 0x6E;
    packet[10]      = 0x82;
    packet[64]      = 0x51;
    packet[65]      = 0x80 | (unsigned char)length;
    memcpy(packet + 66, data, length);

    unsigned char checksum = 0x6E;
    for(size_t i = 64; i < length + 66; i++)
    {
        checksum ^= packet[i];
    }
    packet[length + 66] = checksum;

    bool result = SendControlPacket(packet, sizeof(packet));

    /*-----------------------------------------------------*\
    | Commands succeeds without a sleep, but device can miss|
    | its color change if you do multiple command at a time.|
    | It worked in 50ms but putting 200ms here to be safe.  |
    | This isn't particularly fast as it blocks GUI!        |
    \*-----------------------------------------------------*/
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return result;
}

bool AlienwareAW2725QFController::SetMode(unsigned char mode, unsigned char led_mask, RGBColor color1, RGBColor color2, unsigned char brightness, unsigned char speed)
{
    if(led_mask == 0 || (led_mask & ~AW2725QF_ZONE_ALL) != 0 || speed > AW2725QF_SPEED_FAST)
    {
        return false;
    }

    const unsigned char tempos[] = { 0x00, 0x80, 0xFF };
    unsigned char data[12] =
    {
        0xD0, mode, led_mask,
        (unsigned char)RGBGetRValue(color1), (unsigned char)RGBGetGValue(color1), (unsigned char)RGBGetBValue(color1), brightness
    };
    size_t length;

    switch(mode)
    {
        case AW2725QF_MODE_STATIC:
            length = 7;
            break;

        case AW2725QF_MODE_PULSE:
        case AW2725QF_MODE_BREATHING:
            data[7] = tempos[speed];
            length = 8;
            break;

        case AW2725QF_MODE_MORPH:
            data[6]  = (unsigned char)RGBGetRValue(color2);
            data[7]  = (unsigned char)RGBGetGValue(color2);
            data[8]  = (unsigned char)RGBGetBValue(color2);
            data[9]  = brightness;
            data[10] = 0x02;
            data[11] = tempos[speed];
            length   = 12;
            break;

        case AW2725QF_MODE_SPECTRUM:
            /*-------------------------------------------------*\
            | Spectrum mode only applies to all zones.          |
            | It overwrites other zones even if you specifty a  |
            | zone in command. Also, if you try to change a     |
            | zone out of spectrum mode it stops all other zone |
            | as well.                                          |
            \*-------------------------------------------------*/
            data[2] = AW2725QF_BRIGHTNESS_MAX;
            data[3] = tempos[speed];
            length  = 4;
            break;

        default:
            return false;
    }

    return SendDDCCommand(data, length);
}

std::vector<unsigned char> AlienwareAW2725QFController::GenerateKey(
    const std::vector<unsigned char> &response)
{
    std::vector<unsigned char> syn_key(8, 0);

    const std::vector<unsigned char> oem_key = {
        0x1D, 0xC6, 0x32, 0x5A, 0x3F, 0xD1, 0x85, 0xCD, 0x57, 0x1A, 0xED, 0xB8, 0x62
    };

    size_t sk_idx = 0, ok_idx = 0;
    while(ok_idx < oem_key.size() && sk_idx < 8)
    {
        unsigned char ok_sub_len = (oem_key[ok_idx] & 1) + ((oem_key[ok_idx] & 0x10) >> 4);

        for(unsigned int i = 0; i < ok_sub_len && sk_idx < 8 && ok_idx + 1 < oem_key.size(); i++)
        {
            syn_key[sk_idx] = oem_key[ok_idx + 1] ^ oem_key[ok_idx];
            ok_idx++;
            sk_idx++;
        }
        ok_idx++;
    }

    std::vector<unsigned char> out_buffer;

    uint16_t v15 = static_cast<uint16_t>(response[15]) |
                   (static_cast<uint16_t>(response[0]) << 8);
    bool parity_odd = (std::bitset<16>(v15).count() % 2 != 0);

    if(parity_odd)
    {
        size_t end = std::min<size_t>(8, response.size());
        out_buffer = std::vector<unsigned char>(response.begin(), response.begin() + end);

        if(response.size() > 14)
        {
            unsigned char idx = response[14] & 0x07;
            if((idx + 8) < (unsigned char)response.size())
            {
                out_buffer[idx] ^= response[idx + 8];
            }
        }
    }
    else
    {
        size_t start = std::min<size_t>(8, response.size());
        size_t end = std::min<size_t>(start + 8, response.size());
        out_buffer = std::vector<unsigned char>(response.begin() + start, response.begin() + end);

        if(response.size() > 6)
        {
            unsigned char idx = response[6] & 0x07;
            if(idx < response.size())
            {
                out_buffer[idx] ^= response[idx];
            }
        }
    }

    for(size_t i = 0; i < 8 && i < out_buffer.size(); i++)
    {
        syn_key[i] ^= out_buffer[i];
    }

    return syn_key;
}

AlienwareAW2725QFController::~AlienwareAW2725QFController()
{
    if(dev)
    {
        hid_close(dev);
        dev = nullptr;
    }
}

std::string AlienwareAW2725QFController::GetLocation()
{
    return "HID: " + location;
}

std::string AlienwareAW2725QFController::GetSerialString()
{
    wchar_t serial[256];
    if(hid_get_serial_number_string(dev, serial, 256) >= 0)
    {
        std::wstring ws(serial);
        return StringUtils::wstring_to_string(ws);
    }
    return "";
}
