/*---------------------------------------------------------*\
| ChilkeyKeyboardController.cpp                             |
|                                                           |
|   Driver for Chilkey Slice75 HE keyboard                  |
|                                                           |
|   Aaron Coyco                                 28 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <algorithm>
#include <cstring>
#include <chrono>
#include <thread>
#include "ChilkeyKeyboardController.h"
#include "LogManager.h"
#include "StringUtils.h"

using namespace std::chrono_literals;

/*-----------------------------------------------------*\
| The firmware requires seven palette colours to be sent |
| along with the mode-select packet even while in Custom |
| Per-Key mode.  They are not rendered in that mode, but |
| Chilkey's Web Driver always supplies them, so the same |
| fixed palette is reused here for protocol compatibility |
\*-----------------------------------------------------*/
static const unsigned char chilkey_fallback_palette[CHILKEY_PALETTE_COLOR_COUNT][3] =
{
    { 0xFF, 0x00, 0x00 },
    { 0xFF, 0xFF, 0x00 },
    { 0x00, 0xFF, 0x00 },
    { 0x00, 0xFF, 0xFF },
    { 0x00, 0x00, 0xFF },
    { 0xFF, 0x00, 0xFF },
    { 0xFF, 0xFF, 0xFF }
};

ChilkeyKeyboardController::ChilkeyKeyboardController(hid_device* dev_handle, hid_device_info* info, std::string dev_name)
{
    dev                 = dev_handle;
    location            = info->path;
    name                = dev_name;

    LOG_INFO("[Chilkey Slice75 HE] Found device \"%s\" at %s", name.c_str(), location.c_str());
}

ChilkeyKeyboardController::~ChilkeyKeyboardController()
{
    hid_close(dev);
}

std::string ChilkeyKeyboardController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string ChilkeyKeyboardController::GetNameString()
{
    return(name);
}

std::string ChilkeyKeyboardController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

/*-----------------------------------------------------*\
| Checksum is the low byte of:                           |
|   0x35 + header + length + command + last payload byte |
\*-----------------------------------------------------*/
unsigned char ChilkeyKeyboardController::CalculateChecksum(unsigned char* packet)
{
    unsigned char length    = packet[1];
    unsigned int  checksum  = CHILKEY_CHECKSUM_BASE + packet[0] + length + packet[2];

    if(length > 0 && length <= 0xFC)
    {
        checksum += packet[length + 3];
    }

    return((unsigned char)(checksum & 0xFF));
}

bool ChilkeyKeyboardController::SendPacket(unsigned char* packet)
{
    unsigned char usb_buf[CHILKEY_REPORT_SIZE + 1];

    usb_buf[0] = CHILKEY_REPORT_ID;
    memcpy(&usb_buf[1], packet, CHILKEY_REPORT_SIZE);

    int result = hid_write(dev, usb_buf, sizeof(usb_buf));

    if(result < 0)
    {
        LOG_ERROR("[Chilkey Slice75 HE] Failed to write HID report (command 0x%02X): %ls", packet[2], hid_error(dev));
        return(false);
    }

    return(true);
}

bool ChilkeyKeyboardController::SendSync()
{
    unsigned char packet[CHILKEY_REPORT_SIZE];
    memset(packet, 0x00, sizeof(packet));

    unsigned int offset = 4;

    packet[0]       = CHILKEY_PROTOCOL_HEADER;
    packet[2]       = CHILKEY_COMMAND_SYNC;
    packet[offset++] = 0x01;
    packet[offset++] = 0x02;
    packet[offset++] = 0x03;
    packet[offset++] = 0x04;
    packet[offset++] = 0xFF;
    packet[offset++] = 0xFF;

    packet[1] = (unsigned char)(offset - 4);
    packet[3] = CalculateChecksum(packet);

    return(SendPacket(packet));
}

bool ChilkeyKeyboardController::SendPerKeyLightingMode()
{
    unsigned char packet[CHILKEY_REPORT_SIZE];
    memset(packet, 0x00, sizeof(packet));

    unsigned int offset = 4;

    packet[0]        = CHILKEY_PROTOCOL_HEADER;
    packet[2]        = CHILKEY_COMMAND_LIGHTING;
    packet[offset++] = CHILKEY_SUBCOMMAND_WRITE;

    /*-----------------------------------------*\
    |  Reserved bytes used by the Chilkey Web   |
    |  Driver                                   |
    \*-----------------------------------------*/
    packet[offset++] = 0x00;
    packet[offset++] = 0x00;
    packet[offset++] = 0x00;
    packet[offset++] = 0x00;

    /*-----------------------------------------*\
    |  PRGB palettes are encoded B, G, R,       |
    |  followed by a fixed alpha byte           |
    \*-----------------------------------------*/
    for(unsigned int color_idx = 0; color_idx < CHILKEY_PALETTE_COLOR_COUNT; color_idx++)
    {
        packet[offset++] = chilkey_fallback_palette[color_idx][2];
        packet[offset++] = chilkey_fallback_palette[color_idx][1];
        packet[offset++] = chilkey_fallback_palette[color_idx][0];
        packet[offset++] = 0xFF;
    }

    packet[offset++] = 0x00;
    packet[offset++] = 0x00;
    packet[offset++] = 0x00;
    packet[offset++] = 0x00;

    packet[offset++] = 0x01;                       // Lighting enabled; normal direction; no super-response
    packet[offset++] = CHILKEY_BRIGHTNESS_MAX;
    packet[offset++] = CHILKEY_PER_KEY_LIGHTING_MODE;
    packet[offset++] = 0x00;                       // Effect speed is unused in custom per-key mode
    packet[offset++] = 0x00;                       // Never sleep due to a firmware lighting timeout
    packet[offset++] = 0x00;                       // First static-palette slot; unused in custom per-key mode

    packet[1] = (unsigned char)(offset - 4);
    packet[3] = CalculateChecksum(packet);

    return(SendPacket(packet));
}

void ChilkeyKeyboardController::Initialize()
{
    if(!SendSync())
    {
        LOG_WARNING("[Chilkey Slice75 HE] SYNC command failed; device may not respond to lighting commands");
    }

    std::this_thread::sleep_for(5ms);

    if(!SendPerKeyLightingMode())
    {
        LOG_WARNING("[Chilkey Slice75 HE] Failed to switch device to Custom Per-Key mode");
    }

    std::this_thread::sleep_for(5ms);
}

void ChilkeyKeyboardController::SetColors(const std::vector<RGBColor>& colors, const std::vector<unsigned char>& key_ids)
{
    unsigned int led_count = (unsigned int)colors.size();

    for(unsigned int start_idx = 0; start_idx < led_count; start_idx += CHILKEY_MAX_LEDS_PER_PACKET)
    {
        unsigned int batch_size = std::min<unsigned int>(CHILKEY_MAX_LEDS_PER_PACKET, led_count - start_idx);

        unsigned char packet[CHILKEY_REPORT_SIZE];
        memset(packet, 0xFF, sizeof(packet));

        unsigned int offset = 4;

        packet[0]        = CHILKEY_PROTOCOL_HEADER;
        packet[2]        = CHILKEY_COMMAND_PER_KEY_RGB;
        packet[offset++] = CHILKEY_SUBCOMMAND_WRITE;

        for(unsigned int led_idx = start_idx; led_idx < start_idx + batch_size; led_idx++)
        {
            packet[offset++] = key_ids[led_idx];
            packet[offset++] = RGBGetRValue(colors[led_idx]);
            packet[offset++] = RGBGetGValue(colors[led_idx]);
            packet[offset++] = RGBGetBValue(colors[led_idx]);
        }

        packet[1] = (unsigned char)(offset - 4);
        packet[3] = CalculateChecksum(packet);

        SendPacket(packet);

        if(start_idx + batch_size < led_count)
        {
            std::this_thread::sleep_for(1ms);
        }
    }
}
