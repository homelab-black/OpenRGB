
/*---------------------------------------------------------*\
|   ThermaltakeArgentMP1Controller.cpp                      |
|                                                           |
|   Controller class for Thermaltake Argent MP1 Mousepad    |
|                                                           |
|   John Scripter                                1 Nov 2026 |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "ThermaltakeArgentMP1Controller.h"
#include <cstring>

ThermaltakeArgentMP1Controller::ThermaltakeArgentMP1Controller(hid_device* dev_handle, const hid_device_info& info)
{
    dev      = dev_handle;
    location = info.path;

    if(info.serial_number)
    {
        std::wstring wserial = info.serial_number;
        serial = std::string(wserial.begin(), wserial.end());
    }
}

ThermaltakeArgentMP1Controller::~ThermaltakeArgentMP1Controller()
{
    hid_close(dev);
}

std::string ThermaltakeArgentMP1Controller::GetDeviceLocation()
{
    return location;
}

std::string ThermaltakeArgentMP1Controller::GetSerialString()
{
    return serial;
}

void ThermaltakeArgentMP1Controller::SendInitPacket()
{
    unsigned char buffer[64];
    std::memset(buffer, 0, sizeof(buffer));

    buffer[0] = 0x41; // Magic init byte
    buffer[1] = 0x03;

    hid_write(dev, buffer, 64);
}

void ThermaltakeArgentMP1Controller::SetLedColors(const std::vector<RGBColor>& colors)
{
    // Send initialization command prior to streaming color buffers
    SendInitPacket();

    // --- Packet 1: Handles LEDs 0 to 14 (15 total) ---
    unsigned char p1[64];
    std::memset(p1, 0, sizeof(p1));
    p1[0] = 0xC0; // Magic byte
    p1[1] = 0x01;
    p1[2] = 0x0F; // 15 LEDs addressed
    p1[3] = 0x00;

    int offset = 4;
    for(int i = 0; i < 15; i++)
    {
        p1[offset++] = i;
        p1[offset++] = RGBGetRValue(colors[i]);
        p1[offset++] = RGBGetGValue(colors[i]);
        p1[offset++] = RGBGetBValue(colors[i]);
    }
    hid_write(dev, p1, 64);

    // --- Packet 2: Handles LEDs 15 to 19 (5 total) ---
    unsigned char p2[64];
    // Mirror packet 1 structural pattern to satisfy the controller fill-requirement
    std::memcpy(p2, p1, sizeof(p2));
    p2[0] = 0xC0;
    p2[1] = 0x01;
    p2[2] = 0x05; // 5 remaining LEDs addressed
    p2[3] = 0x00;

    offset = 4;
    for(int i = 15; i < 20; i++)
    {
        p2[offset++] = i;
        p2[offset++] = RGBGetRValue(colors[i]);
        p2[offset++] = RGBGetGValue(colors[i]);
        p2[offset++] = RGBGetBValue(colors[i]);
    }
    // Remaining bytes are inherently left padded from the memcpy layout copy
    hid_write(dev, p2, 64);
}
