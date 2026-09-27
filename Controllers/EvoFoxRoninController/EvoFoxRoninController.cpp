/*---------------------------------------------------------*\
| EvoFoxRoninController.cpp                                 |
|                                                           |
|   Driver for EvoFox Ronin TKL RGB keyboard                |
|                                                           |
|   Protocol reverse engineered from USB HID traffic.       |
|                                                           |
|   <Atif Usmani> (<AtifUsmani>)               <30 09 2026> |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "EvoFoxRoninController.h"
#include "LogManager.h"

EvoFoxRoninController::EvoFoxRoninController(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev      = dev_handle;
    location = path;
    name     = dev_name;
}

EvoFoxRoninController::~EvoFoxRoninController()
{
    hid_close(dev);
}

std::string EvoFoxRoninController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string EvoFoxRoninController::GetDeviceName()
{
    return name;
}

std::string EvoFoxRoninController::GetSerialString()
{
    wchar_t serial_string[128];
    if(hid_get_serial_number_string(dev, serial_string, 128) != 0)
    {
        return "";
    }

    std::string result;
    for(const wchar_t* p = serial_string; *p != L'\0'; ++p)
    {
        if(*p < 0x80)
        {
            result.push_back(static_cast<char>(*p));
        }
    }
    return result;
}

void EvoFoxRoninController::SendReport(const unsigned char* data, size_t length)
{
    unsigned char report[EVOFOX_RONIN_REPORT_SIZE];
    memset(report, 0, sizeof(report));
    if(length > sizeof(report))
    {
        length = sizeof(report);
    }
    memcpy(report, data, length);

    int bytes_written = hid_write(dev, report, sizeof(report));
    if(bytes_written != (int)sizeof(report))
    {
        LOG_ERROR("[EvoFoxRoninController] Failed to write HID report (%d/%d bytes): %ls",
                  bytes_written, (int)sizeof(report), hid_error(dev));
    }
}

void EvoFoxRoninController::SendTransaction(const unsigned char* command, size_t command_length)
{
    // The reverse-engineered protocol uses BEGIN -> COMMAND -> COMMIT.
    const unsigned char begin[]  = { 0x04, 0x01, 0x00, 0x01 };
    const unsigned char commit[] = { 0x04, 0x02, 0x00, 0x02 };

    SendReport(begin, sizeof(begin));
    SendReport(command, command_length);
    SendReport(commit, sizeof(commit));
}

void EvoFoxRoninController::SetLightingConfig(const mode &m, RGBColor color)
{
    const unsigned int effect = m.value;
    const unsigned char command = static_cast<unsigned char>(0x30 + effect);

    // 04 CC 01 06 27 00 00 00 00 EE BB SS DD 00 RR GG BB 05
    unsigned char packet[18] =
    {
        0x04,
        command,
        0x01, 0x06, 0x27,
        0x00, 0x00, 0x00,
        0x00,
        static_cast<unsigned char>(effect),
        static_cast<unsigned char>(m.brightness),
        static_cast<unsigned char>(m.speed),
        static_cast<unsigned char>(m.direction),
        0x00,
        static_cast<unsigned char>(RGBGetRValue(color)),
        static_cast<unsigned char>(RGBGetGValue(color)),
        static_cast<unsigned char>(RGBGetBValue(color)),
        0x05
    };

    SendTransaction(packet, sizeof(packet));
}
