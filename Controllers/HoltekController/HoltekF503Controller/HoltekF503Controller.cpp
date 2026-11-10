/*---------------------------------------------------------*\
| HoltekF503Controller.cpp                                  |
|                                                           |
|   Driver for Holtek mouse                                 |
|                                                           |
|   Remigiusz Pietroń (remememe)                24 Aug 2026 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "HoltekF503Controller.h"
#include "StringUtils.h"

HoltekF503Controller::HoltekF503Controller(hid_device *dev_handle, const char *path, std::string dev_name)
{
    dev         = dev_handle;
    location    = path;
    name        = dev_name;
}

HoltekF503Controller::~HoltekF503Controller()
{
    hid_close(dev);
}

std::string HoltekF503Controller::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string HoltekF503Controller::GetNameString()
{
    return(name);
}

std::string HoltekF503Controller::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

/*-------------------------------------------------------------------------------------------------*\
| Private packet sending functions.                                                                 |
\*-------------------------------------------------------------------------------------------------*/

void HoltekF503Controller::SendCustomColor(
    unsigned char red,
    unsigned char green,
    unsigned char blue,
    unsigned char mode,
    unsigned char speed,
    unsigned char flag)
{
    unsigned char usb_buf[16];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Command header                                       |
    \*-----------------------------------------------------*/
    usb_buf[0]              = 0x02;
    usb_buf[1]              = 0xF3;
    usb_buf[2]              = 0x49;
    usb_buf[3]              = 0x04;
    usb_buf[4]              = 0x06;
    usb_buf[5]              = 0x00;
    usb_buf[6]              = 0x00;
    usb_buf[7]              = 0x00;

    /*-----------------------------------------------------*\
    | RGB control parameters                               |
    \*-----------------------------------------------------*/
    usb_buf[8]              = red;
    usb_buf[9]              = green;
    usb_buf[10]             = blue;
    usb_buf[11]             = mode;
    usb_buf[12]             = speed;
    usb_buf[13]             = flag;
    usb_buf[14]             = 0x00; // padding
    usb_buf[15]             = 0x00; // padding

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));
}

void HoltekF503Controller::SendBrightness(
    unsigned char Brightness
)
{
    unsigned char usb_buf[16];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Command header                                       |
    \*-----------------------------------------------------*/
    usb_buf[0]              = 0x02;
    usb_buf[1]              = 0xF3;
    usb_buf[2]              = 0x4F;
    usb_buf[3]              = 0x04;
    usb_buf[4]              = 0x01;
    usb_buf[5]              = 0x00;
    usb_buf[6]              = 0x00;
    usb_buf[7]              = 0x00;

    /*-----------------------------------------------------*\
    | Brightness control parameters                        |
    \*-----------------------------------------------------*/
    usb_buf[8]              = Brightness;
    usb_buf[9]              = 0x00; // padding
    usb_buf[10]             = 0x00; // padding
    usb_buf[11]             = 0x00; // padding
    usb_buf[12]             = 0x00; // padding
    usb_buf[13]             = 0x00; // padding
    usb_buf[14]             = 0x00; // padding
    usb_buf[15]             = 0x00; // padding

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));
}

void HoltekF503Controller::Apply()
{
    unsigned char usb_buf[16];

    /*-----------------------------------------------------*\
    | Zero out buffer                                       |
    \*-----------------------------------------------------*/
    memset(usb_buf, 0x00, sizeof(usb_buf));

    /*-----------------------------------------------------*\
    | Set up packet                                         |
    \*-----------------------------------------------------*/
    usb_buf[0]  = 0x02;
    usb_buf[1]  = 0xF1;
    usb_buf[2]  = 0x02;
    usb_buf[3]  = 0x04;
    usb_buf[4]  = 0x00;
    usb_buf[5]  = 0x00;
    usb_buf[6]  = 0x00;
    usb_buf[7]  = 0x00;
    usb_buf[8]  = 0x00;
    usb_buf[9]  = 0x00;
    usb_buf[10] = 0x00;
    usb_buf[11] = 0x00;
    usb_buf[12] = 0x00;
    usb_buf[13] = 0x00;
    usb_buf[14] = 0x00;
    usb_buf[15] = 0x00;

    /*-----------------------------------------------------*\
    | Send packet                                           |
    \*-----------------------------------------------------*/
    hid_send_feature_report(dev, usb_buf, sizeof(usb_buf));
}
