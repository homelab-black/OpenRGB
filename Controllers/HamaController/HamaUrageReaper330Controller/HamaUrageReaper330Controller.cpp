/*---------------------------------------------------------*\
| HamaUrageReaper330Controller.cpp                          |
|                                                           |
|   Driver for Hama uRage Reaper 330 mouse                  |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <chrono>
#include <cstring>
#include <thread>
#include "HamaUrageReaper330Controller.h"
#include "StringUtils.h"

/*---------------------------------------------------------*\
| Protocol notes (from a USB capture of the vendor software)|
|                                                           |
| Interface 2 (vendor HID) is used for configuration.       |
|                                                           |
| Commands are 8-byte HID feature reports with report ID 0. |
| For all commands except the mode command, byte 7 is a     |
| checksum such that the sum of all 8 bytes is 0xFF.        |
|                                                           |
| Changing the lighting mode takes four steps:              |
|   1. Feature report  02 02 00 00 00 00 00 FB              |
|   2. Feature report  07 00 MM SS BB 00 00 00              |
|                      (MM=mode, SS=speed, BB=brightness)   |
|   3. Feature report  0E 00 18 00 00 00 00 D9              |
|   4. 64-byte interrupt OUT packet  RR GG BB 00 00 ...     |
|                      (colour, plain 0-255 values)         |
\*---------------------------------------------------------*/

#define HAMA_URAGE_REAPER_330_COMMAND_DELAY_MS     20

HamaUrageReaper330Controller::HamaUrageReaper330Controller(hid_device* dev_handle, const char* path, std::string dev_name)
{
    dev         = dev_handle;
    location    = path;
    name        = dev_name;
}

HamaUrageReaper330Controller::~HamaUrageReaper330Controller()
{
    hid_close(dev);
}

std::string HamaUrageReaper330Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string HamaUrageReaper330Controller::GetNameString()
{
    return(name);
}

std::string HamaUrageReaper330Controller::GetSerialString()
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
| Sends one 8-byte checksummed command as a feature report. |
| Layout: [cmd, index, value, 0, 0, 0, 0, checksum] where   |
| the checksum makes the sum of all 8 bytes equal 0xFF.     |
\*---------------------------------------------------------*/
void HamaUrageReaper330Controller::SendCommand(unsigned char cmd, unsigned char index, unsigned char value)
{
    unsigned char usb_buf[9];

    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00]   = 0x00;     // Report ID (unnumbered)
    usb_buf[0x01]   = cmd;
    usb_buf[0x02]   = index;
    usb_buf[0x03]   = value;

    unsigned char sum = 0;

    for(unsigned int i = 1; i < 8; i++)
    {
        sum += usb_buf[i];
    }

    usb_buf[0x08]   = (unsigned char)(0xFF - sum);

    hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));
}

/*---------------------------------------------------------*\
| Sends the 64-byte interrupt packet that follows the mode  |
| command.  It carries the colour as R, G, B in its first   |
| three bytes.  The report ID byte (0) is added by hidapi's |
| convention and is not part of the 64 bytes on the wire.   |
\*---------------------------------------------------------*/
void HamaUrageReaper330Controller::SendDataPacket(unsigned char red, unsigned char green, unsigned char blue)
{
    unsigned char usb_buf[65];

    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00]   = 0x00;     // Report ID (unnumbered)
    usb_buf[0x01]   = red;
    usb_buf[0x02]   = green;
    usb_buf[0x03]   = blue;

    hid_write(dev, usb_buf, sizeof(usb_buf));
}

void HamaUrageReaper330Controller::SetMode(unsigned char mode, unsigned char speed, unsigned char brightness,
                                           unsigned char red, unsigned char green, unsigned char blue)
{
    const auto delay = std::chrono::milliseconds(HAMA_URAGE_REAPER_330_COMMAND_DELAY_MS);

    /*-----------------------------------------------------*\
    | Step 1: begin                                         |
    \*-----------------------------------------------------*/
    SendCommand(0x02, 0x02, 0x00);
    std::this_thread::sleep_for(delay);

    /*-----------------------------------------------------*\
    | Step 2: set mode, speed and brightness.  This is the  |
    | one command without a checksum, so it is built by     |
    | hand.  Layout:                                        |
    |   07 00 MODE SPEED BRIGHTNESS 00 00 00                |
    \*-----------------------------------------------------*/
    unsigned char usb_buf[9];
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0x00]   = 0x00;     // Report ID (unnumbered)
    usb_buf[0x01]   = 0x07;
    usb_buf[0x02]   = 0x00;
    usb_buf[0x03]   = mode;
    usb_buf[0x04]   = speed;
    usb_buf[0x05]   = brightness;

    hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));
    std::this_thread::sleep_for(delay);

    /*-----------------------------------------------------*\
    | Step 3: commit                                        |
    \*-----------------------------------------------------*/
    SendCommand(0x0E, 0x00, 0x18);
    std::this_thread::sleep_for(delay);

    /*-----------------------------------------------------*\
    | Step 4: data packet                                   |
    \*-----------------------------------------------------*/
    SendDataPacket(red, green, blue);
}
