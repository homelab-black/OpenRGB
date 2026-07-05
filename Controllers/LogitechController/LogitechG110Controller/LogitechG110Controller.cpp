/*---------------------------------------------------------*\
| LogitechG110Controller.cpp                                |
|                                                           |
|   Driver for Logitech G110 Gaming Keyboard                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include "LogitechG110Controller.h"
#include "StringUtils.h"

LogitechG110Controller::LogitechG110Controller(hid_device* dev_handle, const char* path)
{
    dev         = dev_handle;
    location    = path;
}

LogitechG110Controller::~LogitechG110Controller()
{
    hid_close(dev);
}

std::string LogitechG110Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string LogitechG110Controller::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

void LogitechG110Controller::SetColor(unsigned char red, unsigned char blue, unsigned char brightness)
{
    /*-----------------------------------------------------*\
    | The G110 backlight has only red and blue LEDs.  The   |
    | hardware takes a single byte for the red/blue balance |
    | (0x00 = full red, 0xFF = full blue) and a brightness  |
    | byte, sent as feature report 0x07.                    |
    \*-----------------------------------------------------*/
    unsigned char   color_value = 0x00;
    unsigned int    total       = (unsigned int)red + (unsigned int)blue;

    if(total > 0)
    {
        color_value = (unsigned char)(((unsigned int)blue * 0xFF) / total);
    }

    /*-----------------------------------------------------*\
    | Scale brightness from the 0 - 255 API range down to   |
    | the 0 - 0x20 hardware range.                          |
    \*-----------------------------------------------------*/
    unsigned char   scaled_brightness = (unsigned char)(((unsigned int)brightness * 0x20) / 0xFF);

    unsigned char   usb_buf[5];
    usb_buf[0x00]   = 0x07;
    usb_buf[0x01]   = color_value;
    usb_buf[0x02]   = 0x00;
    usb_buf[0x03]   = 0x00;
    usb_buf[0x04]   = scaled_brightness;

    hid_send_feature_report(dev, usb_buf, 5);
}
