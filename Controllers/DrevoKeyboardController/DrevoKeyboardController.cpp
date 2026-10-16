/*---------------------------------------------------------*\
| DrevoKeyboardController.cpp                               |
|                                                           |
|   Driver for DREVO BladeMaster PRO keyboards              |
|                                                           |
|   T-bond                                      27 Sep 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "DrevoKeyboardController.h"
#include "StringUtils.h"

/*---------------------------------------------------------*\
| Side LED slot of each light bar LED:                      |
| - top and bottom bars -> left to right                    |
| - right and left bars -> bottom to top.                   |
| The keyboard chains the LEDs clockwise from the top left  |
| corner.                                                   |
\*---------------------------------------------------------*/
static const unsigned int drevo_light_bar_slots[DREVO_LIGHT_BAR_COUNT] =
{
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13,
    19, 18, 17, 16, 15,
    34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21,
    35, 36, 37, 38, 39,
};

DrevoKeyboardController::DrevoKeyboardController(hid_device* dev_handle, const hid_device_info& info, std::string dev_name)
{
    dev             = dev_handle;
    location        = info.path;
    name            = dev_name;
}

DrevoKeyboardController::~DrevoKeyboardController()
{
    hid_close(dev);
}

std::string DrevoKeyboardController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string DrevoKeyboardController::GetNameString()
{
    return(name);
}

std::string DrevoKeyboardController::GetSerialString()
{
    wchar_t serial_string[128];
    int     ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

void DrevoKeyboardController::SetMode(unsigned char mode_value, bool use_color, unsigned char speed, unsigned char brightness, RGBColor color, unsigned char direction)
{
    unsigned char report[DREVO_REPORT_SIZE] = { 0 };

    report[0] = DREVO_REPORT_ID;
    report[1] = 0xFE;
    report[2] = mode_value;
    report[3] = ((use_color ? 1 : 0) << 7) | ((speed & 0x07) << 4) | (brightness & 0x0F);

    if(mode_value == DREVO_MODE_RAINBOW)
    {
        report[4] = direction;
    }
    else
    {
        report[4] = RGBGetRValue(color);
        report[5] = RGBGetGValue(color);
        report[6] = RGBGetBValue(color);
    }

    SendReport(report);
}

void DrevoKeyboardController::SetLEDs(const std::vector<RGBColor>& colors)
{
    unsigned char packet[DREVO_LED_PACKET_SIZE] = { 0 };

    packet[0] = 0xF3;
    packet[1] = 0x00;
    packet[2] = 0x00;
    packet[3] = DREVO_LAYOUT_ISO_88;

    /*-----------------------------------------------------*\
    | Side LED data starts right after the keys             |
    \*-----------------------------------------------------*/
    const unsigned int bars_offset = DREVO_LED_PACKET_HEADER_SIZE + (DREVO_ISO_88_KEY_COUNT * 3);

    for(unsigned int led_idx = 0; led_idx < colors.size(); led_idx++)
    {
        unsigned int offset;

        if(led_idx < DREVO_ISO_88_KEY_COUNT)
        {
            offset = DREVO_LED_PACKET_HEADER_SIZE + (led_idx * 3);
        }
        else if(led_idx < DREVO_ISO_88_KEY_COUNT + DREVO_LIGHT_BAR_COUNT)
        {
            offset = bars_offset + (drevo_light_bar_slots[led_idx - DREVO_ISO_88_KEY_COUNT] * 3);
        }
        else
        {
            break;
        }

        packet[offset + 0] = RGBGetRValue(colors[led_idx]);
        packet[offset + 1] = RGBGetGValue(colors[led_idx]);
        packet[offset + 2] = RGBGetBValue(colors[led_idx]);
    }

    SendPacket(packet, sizeof(packet));
}

void DrevoKeyboardController::SendReport(unsigned char* report)
{
    hid_send_feature_report(dev, report, DREVO_REPORT_SIZE);
}

/*---------------------------------------------------------*\
| Split a long packet into reports: 05 d0..d6, then         |
| 05 00 d7..d12, ... One report is sent per 6 bytes of the  |
| packet, so the last report is padded with a zero byte.    |
\*---------------------------------------------------------*/
void DrevoKeyboardController::SendPacket(unsigned char* packet, unsigned int packet_size)
{
    unsigned int reports = packet_size / 6;
    unsigned int pos     = 0;

    for(unsigned int report_idx = 0; report_idx < reports; report_idx++)
    {
        unsigned char report[DREVO_REPORT_SIZE] = { 0 };

        report[0] = DREVO_REPORT_ID;

        for(unsigned int byte_idx = (report_idx == 0) ? 1 : 2; byte_idx < DREVO_REPORT_SIZE; byte_idx++, pos++)
        {
            report[byte_idx] = (pos < packet_size) ? packet[pos] : 0x00;
        }

        SendReport(report);
    }
}
