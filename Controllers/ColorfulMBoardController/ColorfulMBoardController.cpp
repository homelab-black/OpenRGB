/*---------------------------------------------------------*\
| ColorfulMBoardController.cpp                             |
|                                                           |
|   Driver for Colorful USB motherboard lighting            |
|                                                           |
|   OpenRGB contributors                       08 Oct 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "ColorfulMBoardController.h"
#include "LogManager.h"

ColorfulMBoardController::ColorfulMBoardController(hid_device* dev_handle, const char* path, const std::string& dev_name)
{
    dev         = dev_handle;
    location    = path;
    name        = dev_name;
}

ColorfulMBoardController::~ColorfulMBoardController()
{
    hid_close(dev);
}

std::string ColorfulMBoardController::GetDeviceName()
{
    return(name);
}

std::string ColorfulMBoardController::GetDeviceLocation()
{
    return("HID: " + location);
}

bool ColorfulMBoardController::SendPacket(unsigned char command, unsigned char sequence, const RGBColor* colors, unsigned int count)
{
    /*-----------------------------------------------------*\
    | Unnumbered 64-byte output report.  HIDAPI requires a  |
    | leading zero report ID on every platform.            |
    \*-----------------------------------------------------*/
    unsigned char buffer[COLORFUL_MBOARD_REPORT_SIZE] = {};

    buffer[1] = 0x01;
    buffer[3] = command;
    buffer[4] = sequence;

    for(unsigned int led_idx = 0; led_idx < count; led_idx++)
    {
        buffer[5 + led_idx * 3] = RGBGetRValue(colors[led_idx]);
        buffer[6 + led_idx * 3] = RGBGetGValue(colors[led_idx]);
        buffer[7 + led_idx * 3] = RGBGetBValue(colors[led_idx]);
    }

    int bytes_written = hid_write(dev, buffer, sizeof(buffer));

    if(bytes_written != (int)sizeof(buffer))
    {
        LOG_ERROR("[ColorfulMBoardController] Failed to write command 0x%02X, sequence 0x%02X (%d/%u bytes)",
                  command, sequence, bytes_written, (unsigned int)sizeof(buffer));
        return(false);
    }

    return(true);
}

bool ColorfulMBoardController::SendNativePacket(const unsigned char* prefix, unsigned int length)
{
    /*-----------------------------------------------------*\
    | Native mode commands are a single 64-byte output      |
    | report.  The prefix includes the leading zero report  |
    | ID byte required by HIDAPI.                           |
    \*-----------------------------------------------------*/
    unsigned char buffer[COLORFUL_MBOARD_REPORT_SIZE] = {};

    if(length > sizeof(buffer))
    {
        length = sizeof(buffer);
    }

    memcpy(buffer, prefix, length);

    int bytes_written = hid_write(dev, buffer, sizeof(buffer));

    if(bytes_written != (int)sizeof(buffer))
    {
        LOG_ERROR("[ColorfulMBoardController] Failed to write native command 0x%02X (%d/%u bytes)",
                  prefix[3], bytes_written, (unsigned int)sizeof(buffer));
        return(false);
    }

    return(true);
}

void ColorfulMBoardController::SetLEDs(const std::vector<RGBColor>& colors)
{
    /*-----------------------------------------------------*\
    | Stage all 25 windows, clearing unused slots.  The     |
    | final window has only two meaningful RGB triples.    |
    | Do not commit a frame if any of its writes failed.    |
    \*-----------------------------------------------------*/
    unsigned int led_count = COLORFUL_MBOARD_LED_COUNT;

    if(colors.size() < led_count)
    {
        led_count = (unsigned int)colors.size();
    }

    for(unsigned int packet_idx = 0; packet_idx < COLORFUL_MBOARD_DATA_PACKETS; packet_idx++)
    {
        unsigned int    offset        = packet_idx * COLORFUL_MBOARD_LEDS_PER_PACKET;
        unsigned int    count         = 0;
        const RGBColor* packet_colors = nullptr;

        if(offset < led_count)
        {
            count = led_count - offset;
            if(count > COLORFUL_MBOARD_LEDS_PER_PACKET)
            {
                count = COLORFUL_MBOARD_LEDS_PER_PACKET;
            }
            packet_colors = &colors[offset];
        }

        if(!SendPacket(COLORFUL_MBOARD_COMMAND_DIRECT, packet_idx, packet_colors, count))
        {
            return;
        }
    }

    /*-----------------------------------------------------*\
    | The FF sequence ends a Direct frame.  Command AA     |
    | selects Windows Dynamic Lighting, not frame commit. |
    \*-----------------------------------------------------*/
    SendPacket(COLORFUL_MBOARD_COMMAND_DIRECT, COLORFUL_MBOARD_END_SEQUENCE, nullptr, 0);
}

void ColorfulMBoardController::SetSleep()
{
    const unsigned char prefix[] =
    {
        0x00, 0x01, 0x00, COLORFUL_MBOARD_NATIVE_SLEEP
    };

    SendNativePacket(prefix, sizeof(prefix));
}

void ColorfulMBoardController::SetBreathe(RGBColor color, unsigned char speed)
{
    /*-----------------------------------------------------*\
    | Keep the vendor's fixed byte 8 value.  Its exact      |
    | meaning is unknown; the Breathe builder does not use  |
    | the vendor brightness parameter.                     |
    \*-----------------------------------------------------*/
    const unsigned char prefix[] =
    {
        0x00, 0x01, 0x00, COLORFUL_MBOARD_NATIVE_BREATHE,
        (unsigned char)RGBGetRValue(color),
        (unsigned char)RGBGetGValue(color),
        (unsigned char)RGBGetBValue(color),
        0x00, 0xFF, 0x01, 0x00, 0x01, 0x00,
        speed, 0x01, speed, 0x01
    };

    SendNativePacket(prefix, sizeof(prefix));
}

void ColorfulMBoardController::SetCircle(unsigned char brightness, unsigned char speed)
{
    const unsigned char prefix[] =
    {
        0x00, 0x01, 0x00, COLORFUL_MBOARD_NATIVE_CIRCLE,
        0x00, 0x00, 0x68, 0x01,
        brightness, 0x01, (unsigned char)(6 - speed), 0x00
    };

    SendNativePacket(prefix, sizeof(prefix));
}

void ColorfulMBoardController::SetRainbow(unsigned char brightness, unsigned char speed)
{
    const unsigned char prefix[] =
    {
        0x00, 0x01, 0x00, COLORFUL_MBOARD_NATIVE_RAINBOW,
        speed, 0x01, brightness
    };

    SendNativePacket(prefix, sizeof(prefix));
}
