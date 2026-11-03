/*---------------------------------------------------------*\
| CherryMouseController.cpp                                 |
|                                                           |
|   Driver for Cherry mouse                                 |
|                                                           |
|   Rik Vanden Boer                             03 Nov 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "CherryMouseController.h"
#include "StringUtils.h"

CherryMouseController::CherryMouseController(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev         = dev_handle;
    location    = path;
    name        = dev_name;
}

CherryMouseController::~CherryMouseController()
{
    hid_close(dev);
}

std::string CherryMouseController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string CherryMouseController::GetDeviceName()
{
    return(name);
}

std::string CherryMouseController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

/*---------------------------------------------------------*\
| The mouse uses the same packet layout as Cherry           |
| keyboards: report ID 0x04, checksum (bytes 1-2, sum of    |
| bytes 3-63), command, size, parameter, 0x55 marker, data. |
| Every change is wrapped in a begin/end pair, as the       |
| CHERRY Utility does.                                      |
\*---------------------------------------------------------*/
void CherryMouseController::SetMode
    (
    unsigned char       mode,
    unsigned char       brightness,
    unsigned char       speed,
    unsigned char       direction,
    unsigned char       random_flag,
    unsigned char       red,
    unsigned char       green,
    unsigned char       blue
    )
{
    unsigned char usb_buf[CHERRY_MOUSE_PACKET_SIZE];

    SendCommand(CHERRY_MOUSE_COMMAND_BEGIN);

    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00]           = 0x04;
    usb_buf[0x03]           = CHERRY_MOUSE_COMMAND_SET_PARAMETER;
    usb_buf[0x04]           = 0x09;
    usb_buf[0x05]           = 0x00;
    usb_buf[0x07]           = 0x55;
    usb_buf[0x08]           = 0x00;
    usb_buf[0x09]           = mode;
    usb_buf[0x0A]           = brightness;
    usb_buf[0x0B]           = speed;
    usb_buf[0x0C]           = direction;
    usb_buf[0x0D]           = random_flag;
    usb_buf[0x0E]           = red;
    usb_buf[0x0F]           = green;
    usb_buf[0x10]           = blue;

    SendPacket(usb_buf);

    SendCommand(CHERRY_MOUSE_COMMAND_END);
}

void CherryMouseController::SendCommand(unsigned char command)
{
    unsigned char usb_buf[CHERRY_MOUSE_PACKET_SIZE];

    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00]           = 0x04;
    usb_buf[0x03]           = command;
    usb_buf[0x07]           = 0x55;

    SendPacket(usb_buf);
}

void CherryMouseController::SendPacket(unsigned char* usb_buf)
{
    unsigned short checksum = 0;

    for(unsigned int byte_idx = 0x03; byte_idx < CHERRY_MOUSE_PACKET_SIZE; byte_idx++)
    {
        checksum += usb_buf[byte_idx];
    }

    usb_buf[0x01] = checksum & 0xFF;
    usb_buf[0x02] = checksum >> 8;

    hid_write(dev, usb_buf, CHERRY_MOUSE_PACKET_SIZE);
    hid_read_timeout(dev, usb_buf, CHERRY_MOUSE_PACKET_SIZE, CHERRY_MOUSE_READ_TIMEOUT_MS);
}
