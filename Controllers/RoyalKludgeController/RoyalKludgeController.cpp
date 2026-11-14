/*---------------------------------------------------------*\
| RoyalKludgeController.cpp                                 |
|                                                           |
|   Driver for Royal Kludge RK987 keyboards                 |
|                                                           |
|   johnm                                       01 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <algorithm>
#include <cstring>
#include "LogManager.h"
#include "StringUtils.h"
#include "RoyalKludgeController.h"

RoyalKludgeController::RoyalKludgeController(hid_device* dev_handle, const char* path, const std::string& /*dev_name*/)
{
    dev         = dev_handle;
    location    = "HID: ";
    location   += path;
    name        = "Royal Kludge RK987";
}

RoyalKludgeController::~RoyalKludgeController()
{
    hid_close(dev);
}

std::string RoyalKludgeController::GetDeviceLocation()
{
    return(location);
}

std::string RoyalKludgeController::GetNameString()
{
    return(name);
}

std::string RoyalKludgeController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

void RoyalKludgeController::SendBuffer(unsigned char* buffer)
{
    int ret = hid_send_feature_report(dev, buffer, ROYAL_KLUDGE_REPORT_SIZE);

    if(ret != ROYAL_KLUDGE_REPORT_SIZE)
    {
        LOG_ERROR("[%s] Failed to send feature report: %ls", name.c_str(), hid_error(dev));
    }
}

void RoyalKludgeController::SendStandardBuffer(unsigned char mode, unsigned char speed, unsigned char brightness, RGBColor color, bool random_color)
{
    unsigned char buffer[ROYAL_KLUDGE_REPORT_SIZE];

    memset(buffer, 0x00, sizeof(buffer));

    buffer[0]   = 0x0A;
    buffer[1]   = 0x01;
    buffer[2]   = 0x01;
    buffer[3]   = 0x02;
    buffer[4]   = 0x29;
    buffer[5]   = mode;
    buffer[6]   = 0x00;
    buffer[7]   = speed;
    buffer[8]   = brightness;

    if(!random_color)
    {
        buffer[9]   = RGBGetRValue(color);
        buffer[10]  = RGBGetGValue(color);
        buffer[11]  = RGBGetBValue(color);
    }

    buffer[12]  = random_color ? 0x01 : 0x00;
    buffer[13]  = 0x05;

    SendBuffer(buffer);
}

void RoyalKludgeController::SetMode(unsigned char mode, unsigned char speed, unsigned char brightness, RGBColor color, bool random_color)
{
    std::lock_guard<std::mutex> guard(device_mutex);

    SendStandardBuffer(mode, speed, brightness, color, random_color);
}

void RoyalKludgeController::SetLEDsDirect(const std::vector<unsigned int>& led_indices, const std::vector<RGBColor>& colors)
{
    /*---------------------------------------------------------*\
    | The seven custom buffers below both switch the keyboard    |
    | to its per-key (custom) light mode and update the colors.  |
    | Sending the standard mode buffer here would switch the     |
    | keyboard back out of custom mode, so it is intentionally   |
    | not sent.                                                   |
    \*---------------------------------------------------------*/

    std::lock_guard<std::mutex> guard(device_mutex);

    /*---------------------------------------------------------*\
    | Build the flat LED buffer.  Each LED uses three bytes.     |
    \*---------------------------------------------------------*/
    unsigned char led_data[ROYAL_KLUDGE_CUSTOM_DATA_SIZE];

    memset(led_data, 0x00, sizeof(led_data));

    size_t color_count = std::min(led_indices.size(), colors.size());

    for(size_t color_idx = 0; color_idx < color_count; color_idx++)
    {
        unsigned int led_idx = led_indices[color_idx];

        if(led_idx >= (ROYAL_KLUDGE_CUSTOM_DATA_SIZE / 3))
        {
            LOG_WARNING("[%s] LED index %u is outside of the custom LED buffer", name.c_str(), led_idx);
            continue;
        }

        led_data[(led_idx * 3) + 0] = RGBGetRValue(colors[color_idx]);
        led_data[(led_idx * 3) + 1] = RGBGetGValue(colors[color_idx]);
        led_data[(led_idx * 3) + 2] = RGBGetBValue(colors[color_idx]);
    }

    /*---------------------------------------------------------*\
    | Split the LED buffer into seven 65-byte feature reports.   |
    \*---------------------------------------------------------*/
    unsigned int data_idx = 0;

    for(unsigned int buffer_idx = 0; buffer_idx < ROYAL_KLUDGE_CUSTOM_BUFFERS; buffer_idx++)
    {
        unsigned char buffer[ROYAL_KLUDGE_REPORT_SIZE];
        unsigned int  data_start = 3;

        memset(buffer, 0x00, sizeof(buffer));

        buffer[0] = 0x0A;
        buffer[1] = ROYAL_KLUDGE_CUSTOM_BUFFERS;
        buffer[2] = buffer_idx + 1;

        if(buffer_idx == 0)
        {
            buffer[3]   = 0x03;
            buffer[4]   = 0x7E;
            buffer[5]   = 0x01;
            data_start  = 6;
        }

        for(unsigned int buffer_pos = data_start; buffer_pos < ROYAL_KLUDGE_REPORT_SIZE; buffer_pos++)
        {
            if(data_idx >= ROYAL_KLUDGE_CUSTOM_DATA_SIZE)
            {
                break;
            }

            buffer[buffer_pos] = led_data[data_idx++];
        }

        SendBuffer(buffer);
    }
}
