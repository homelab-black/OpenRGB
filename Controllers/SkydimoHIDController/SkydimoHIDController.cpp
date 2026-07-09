/*---------------------------------------------------------*\
| SkydimoHIDController.cpp                                  |
|                                                           |
|   Driver for Skydimo HID devices                          |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "SkydimoHIDController.h"
#include "StringUtils.h"

#define PACKET_SIZE         64
#define PACKET_RGB_BYTES    60

static unsigned char crc8(const unsigned char* data, size_t len)
{
    unsigned char crc = 0x00;
    for (size_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ 0x07;
            }
            else
            {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

SkydimoHIDController::SkydimoHIDController(hid_device* dev_handle, const char* path)
{
    dev         = dev_handle;
    location    = path;

    wchar_t name_string[255];
    int ret;

    ret = hid_get_manufacturer_string(dev, name_string, 255);
    if (ret == 0)
    {
        device_name = StringUtils::wstring_to_string(name_string);
    }
    else
    {
        device_name = "Skydimo";
    }

    ret = hid_get_product_string(dev, name_string, 255);
    if (ret == 0)
    {
        device_name.append(" ").append(StringUtils::wstring_to_string(name_string));
    }
    else
    {
        device_name.append(" HID Device");
    }

    wchar_t serial_string[128];
    ret = hid_get_serial_number_string(dev, serial_string, 128);
    if (ret == 0)
    {
        serial = StringUtils::wstring_to_string(serial_string);
    }
    else
    {
        serial = "";
    }
}

SkydimoHIDController::~SkydimoHIDController()
{
    if (dev)
    {
        hid_close(dev);
    }
}

std::string SkydimoHIDController::GetDeviceName()
{
    return device_name;
}

std::string SkydimoHIDController::GetSerial()
{
    return serial;
}

std::string SkydimoHIDController::GetLocation()
{
    return location;
}

void SkydimoHIDController::SendPacket(unsigned char start_led, const unsigned char* chunk, size_t chunk_len)
{
    unsigned char buf[64];
    memset(buf, 0x00, sizeof(buf));

    buf[0] = 0x01; // Report ID 1
    buf[1] = start_led;
    buf[2] = 0x00;

    if (chunk_len > 60)
    {
        chunk_len = 60;
    }

    memcpy(&buf[3], chunk, chunk_len);

    // Calculate CRC8 over the first 63 bytes of the report (buf[0] to buf[62])
    buf[63] = crc8(buf, 63);

    hid_write(dev, buf, 64);
}

void SkydimoHIDController::SetLEDs(const std::vector<RGBColor>& colors)
{
    // Frame size: 49 LEDs * 3 = 147 bytes in GRB format
    unsigned char frame_data[147];
    memset(frame_data, 0, sizeof(frame_data));

    // Convert colors to GRB format
    for (size_t i = 0; i < colors.size() && i < 49; i++)
    {
        frame_data[i * 3 + 0] = RGBGetGValue(colors[i]);
        frame_data[i * 3 + 1] = RGBGetRValue(colors[i]);
        frame_data[i * 3 + 2] = RGBGetBValue(colors[i]);
    }

    // Send the frame in 3 packets
    // Packet 0: starts at 0, length 60
    SendPacket(0, &frame_data[0], 60);

    // Packet 1: starts at 20 (which is index 60), length 60
    SendPacket(20, &frame_data[60], 60);

    // Packet 2: starts at 40 (which is index 120), length 27
    SendPacket(40, &frame_data[120], 27);

    // Send the end packet
    unsigned char end_buf[64];
    memset(end_buf, 0x00, sizeof(end_buf));

    end_buf[0] = 0x01; // Report ID 1
    end_buf[1] = 0xFF;
    end_buf[2] = 0xFF;
    end_buf[3] = 0x31;
    end_buf[60] = 0x1E;

    hid_write(dev, end_buf, 64);
}
