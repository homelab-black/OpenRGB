/*---------------------------------------------------------*\
| SteelSeriesMSIRGBController.cpp                           |
|                                                           |
|   Driver for MSI laptop SteelSeries KLC/ALC RGB devices   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "SteelSeriesMSIRGBController.h"
#include "StringUtils.h"

#define STEELSERIES_MSI_RGB_REPORT_ID       0x00
#define STEELSERIES_MSI_RGB_COMMAND         0x0C
#define STEELSERIES_MSI_RGB_KLC_PACKET_ID   0x66
#define STEELSERIES_MSI_RGB_ALC_PACKET_ID   0x06
#define STEELSERIES_MSI_RGB_PACKET_SIZE     525
#define STEELSERIES_MSI_RGB_PAYLOAD_OFFSET  5

SteelSeriesMSIRGBController::SteelSeriesMSIRGBController(hid_device* dev_handle, const char* path, std::string dev_name, steelseries_msi_rgb_device device_type)
{
    dev         = dev_handle;
    location    = path;
    name        = dev_name;
    type        = device_type;
}

SteelSeriesMSIRGBController::~SteelSeriesMSIRGBController()
{
    hid_close(dev);
}

std::string SteelSeriesMSIRGBController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string SteelSeriesMSIRGBController::GetDeviceName()
{
    return(name);
}

std::string SteelSeriesMSIRGBController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

steelseries_msi_rgb_device SteelSeriesMSIRGBController::GetDeviceType()
{
    return(type);
}

void SteelSeriesMSIRGBController::SetLEDs(std::vector<led> leds, std::vector<RGBColor> colors)
{
    unsigned char buf[STEELSERIES_MSI_RGB_PACKET_SIZE];
    unsigned int led_count = (leds.size() < colors.size()) ? leds.size() : colors.size();

    memset(buf, 0x00, sizeof(buf));

    buf[0x00] = STEELSERIES_MSI_RGB_REPORT_ID;
    buf[0x01] = STEELSERIES_MSI_RGB_COMMAND;
    buf[0x03] = (type == STEELSERIES_MSI_RGB_KLC) ? STEELSERIES_MSI_RGB_KLC_PACKET_ID : STEELSERIES_MSI_RGB_ALC_PACKET_ID;

    for(unsigned int led_idx = 0; led_idx < led_count; led_idx++)
    {
        unsigned int offset = STEELSERIES_MSI_RGB_PAYLOAD_OFFSET + (led_idx * 4);

        if((offset + 3) >= sizeof(buf))
        {
            break;
        }

        buf[offset + 0] = leds[led_idx].value;
        buf[offset + 1] = RGBGetRValue(colors[led_idx]);
        buf[offset + 2] = RGBGetGValue(colors[led_idx]);
        buf[offset + 3] = RGBGetBValue(colors[led_idx]);
    }

    hid_send_feature_report(dev, buf, sizeof(buf));
}
