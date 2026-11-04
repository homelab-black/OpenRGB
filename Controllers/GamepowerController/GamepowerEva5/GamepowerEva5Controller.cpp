/*---------------------------------------------------------*\
|   GamepowerEva5Controller.cpp                             |
|                                                           |
|   Driver for Gamepower Eva 5 keyboard                     |
|                                                           |
|   Made by FeusX  (23 Aug 2026)                            |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/


#include "GamepowerEva5Controller.h"

#define GAMEPOWER_EVA5_PAYLOAD_SIZE 8
#define GAMEPOWER_EVA5_REPORT_ID   0x07
#define GAMEPOWER_EVA5_HEADER_1    0xFF
#define GAMEPOWER_EVA5_HEADER_2    0xFF

GamepowerEva5Controller::GamepowerEva5Controller(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev      = dev_handle;
    location = path;
    name     = dev_name;
}

GamepowerEva5Controller::~GamepowerEva5Controller()
{
    hid_close(dev);
}

std::string GamepowerEva5Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string GamepowerEva5Controller::GetDeviceName()
{
    return(name);
}

std::string GamepowerEva5Controller::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    std::wstring return_wstring = serial_string;
    return(std::string(return_wstring.begin(), return_wstring.end()));
}

void GamepowerEva5Controller::SetMode(unsigned char mode, unsigned char brig, unsigned char speed)
{
    uint8_t payload[GAMEPOWER_EVA5_PAYLOAD_SIZE] = {
        GAMEPOWER_EVA5_REPORT_ID, GAMEPOWER_EVA5_HEADER_1, GAMEPOWER_EVA5_HEADER_2,
        mode, brig, speed, 0x00, 0x00
    };

    SendPayload(payload, GAMEPOWER_EVA5_PAYLOAD_SIZE);
}

void GamepowerEva5Controller::SendPayload(const unsigned char* data, short length)
{
    hid_write(dev, data, length);
}
