/*---------------------------------------------------------*\
| CMARGBStripController.cpp                                 |
|                                                           |
|   Driver for Cooler Master ARGB light strip devices       |
|                                                           |
|   Morgan Guimard (morg)                       18 Sep 2023 |
|   Logan Phillips (Eclipse)                    05 Nov 2025 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "CMARGBStripController.h"
#include "StringUtils.h"

static const uint8_t broadcast_strip_id[] = { 0x03 };

CMARGBStripController::CMARGBStripController(hid_device* dev_handle, const hid_device_info& info, const std::string& name, const CMARGBStripConfig& device_config, bool reset, bool broadcast)
{
    dev                      = dev_handle;
    device_name              = name;
    location                 = info.path;
    config                   = device_config;
    is_software_mode_enabled = false;

    /*---------------------------------------------------------*\
    | Override config to use broadcast mode if enabled          |
    \*---------------------------------------------------------*/

    if(broadcast && config.num_strips > 1)
    {
        config.strip_ids  = broadcast_strip_id;
        config.num_strips = 1;
    }

    if(reset)
    {
        ResetDevice();
    }
}

CMARGBStripController::~CMARGBStripController()
{
    hid_close(dev);
}

std::string CMARGBStripController::GetDeviceName()
{
    return device_name;
}

std::string CMARGBStripController::GetDeviceLocation()
{
    return "HID: " + location;
}

std::string CMARGBStripController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return "";
    }

    return StringUtils::wstring_to_string(serial_string);
}

unsigned int CMARGBStripController::GetLEDCount()
{
    return config.leds_per_strip * config.num_strips;
}

unsigned int CMARGBStripController::GetStripCount()
{
    return config.num_strips;
}

/*---------------------------------------------------------*\
| SetMode: Set hardware effect mode                         |
\*---------------------------------------------------------*/

void CMARGBStripController::SetMode(uint8_t mode_value, uint8_t speed, uint8_t brightness, const RGBColor& color, bool random)
{

    uint8_t usb_buf[CM_ARGB_STRIP_PACKET_LENGTH];

    for(unsigned int i = 0; i < config.num_strips; i++)
    {
        uint8_t strip_id = config.strip_ids[i];

        memset(usb_buf, 0x00, CM_ARGB_STRIP_PACKET_LENGTH);

        usb_buf[1]  = 0x80;
        usb_buf[2]  = 0x0B;
        usb_buf[3]  = 0x02;
        usb_buf[4]  = strip_id;
        usb_buf[5]  = mode_value;
        usb_buf[6]  = random ? 0x00 : 0x08;
        usb_buf[7]  = speed;
        usb_buf[8]  = brightness;
        usb_buf[9]  = RGBGetRValue(color);
        usb_buf[10] = RGBGetGValue(color);
        usb_buf[11] = RGBGetBValue(color);

        hid_write(dev, usb_buf, CM_ARGB_STRIP_PACKET_LENGTH);
    }

    if(is_software_mode_enabled)
    {
        SetControlMode(false);
    }

}

/*---------------------------------------------------------*\
| SendColorData: Send color data for Direct or Custom mode  |
\*---------------------------------------------------------*/

void CMARGBStripController::SendColorData(const std::vector<RGBColor>& colors, uint8_t command, uint8_t mode_byte, uint8_t brightness, bool desired_control_mode)
{
    /*---------------------------------------------------------*\
    | For software mode, must enable control mode BEFORE data   |
    | For hardware mode, must send data BEFORE disabling        |
    \*---------------------------------------------------------*/

    if(desired_control_mode && !is_software_mode_enabled)
    {
        SetControlMode(true);
    }

    const unsigned int strip_data_length = CM_ARGB_STRIP_FIRST_PACKET_DATA_SIZE + (CM_ARGB_STRIP_PACKET_DATA_SIZE * 6);
    const unsigned int total_data_length = strip_data_length * config.num_strips;

    uint8_t* color_data = new uint8_t[total_data_length];
    memset(color_data, 0x00, total_data_length);

    /*---------------------------------------------------------*\
    | Fill color buffer for all strips                          |
    \*---------------------------------------------------------*/

    for(unsigned int i = 0; i < colors.size() && i < GetLEDCount(); i++)
    {
        unsigned int strip            = i / config.leds_per_strip;
        unsigned int led_in_strip     = i % config.leds_per_strip;
        unsigned int buffer_offset    = (strip * strip_data_length) + (led_in_strip * 3);

        color_data[buffer_offset]     = RGBGetRValue(colors[i]);
        color_data[buffer_offset + 1] = RGBGetGValue(colors[i]);
        color_data[buffer_offset + 2] = RGBGetBValue(colors[i]);
    }

    uint8_t usb_buf[CM_ARGB_STRIP_PACKET_LENGTH];

    /*---------------------------------------------------------*\
    | Send packets for each strip                               |
    \*---------------------------------------------------------*/

    for(unsigned int i = 0; i < config.num_strips; i++)
    {
        uint8_t strip_id = config.strip_ids[i];
        unsigned int offset = i * strip_data_length;

        for(unsigned int packet = 0; packet < 7; packet++)
        {
            memset(usb_buf, 0x00, CM_ARGB_STRIP_PACKET_LENGTH);

            usb_buf[1] = (packet < 6) ? packet : 0x86;

            /*---------------------------------------------------------*\
            | First packet contains static data                         |
            \*---------------------------------------------------------*/

            if(packet == 0)
            {
                usb_buf[2] = command;
                usb_buf[3] = 0x02;
                usb_buf[4] = strip_id;
                usb_buf[5] = mode_byte;
                usb_buf[6] = brightness;

                memcpy(&usb_buf[7], &color_data[offset], CM_ARGB_STRIP_FIRST_PACKET_DATA_SIZE);
                offset += CM_ARGB_STRIP_FIRST_PACKET_DATA_SIZE;
            }
            else
            {
                memcpy(&usb_buf[2], &color_data[offset], CM_ARGB_STRIP_PACKET_DATA_SIZE);
                offset += CM_ARGB_STRIP_PACKET_DATA_SIZE;
            }

            hid_write(dev, usb_buf, CM_ARGB_STRIP_PACKET_LENGTH);
        }
    }

    delete[] color_data;

    if(!desired_control_mode && is_software_mode_enabled)
    {
        SetControlMode(false);
    }
}

/*---------------------------------------------------------*\
| SetControlMode: Enable/disable software control           |
| True enables software mode                                |
| False enables hardware mode                               |
\*---------------------------------------------------------*/

void CMARGBStripController::SetControlMode(bool software_mode)
{
    uint8_t usb_buf[CM_ARGB_STRIP_PACKET_LENGTH];

    for(unsigned int i = 0; i < config.num_strips; i++)
    {
        uint8_t strip_id = config.strip_ids[i];

        memset(usb_buf, 0x00, CM_ARGB_STRIP_PACKET_LENGTH);

        usb_buf[1]  = 0x80;
        usb_buf[2]  = 0x07;
        usb_buf[3]  = 0x02;
        usb_buf[4]  = strip_id;
        usb_buf[6]  = software_mode;

        hid_write(dev, usb_buf, CM_ARGB_STRIP_PACKET_LENGTH);
    }

    is_software_mode_enabled = software_mode;
}

/*---------------------------------------------------------*\
| ResetDevice: Reset device on discovery                    |
\*---------------------------------------------------------*/

void CMARGBStripController::ResetDevice()
{
    uint8_t usb_buf[CM_ARGB_STRIP_PACKET_LENGTH];

    memset(usb_buf, 0x00, CM_ARGB_STRIP_PACKET_LENGTH);
    usb_buf[1] = 0x80;
    usb_buf[2] = 0x11;
    hid_write(dev, usb_buf, CM_ARGB_STRIP_PACKET_LENGTH);

    is_software_mode_enabled = false;
}
